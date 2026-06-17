/*
 * SequenceBits CUDA benchmark note, 2026-05-04:
 * Compared the original packed reference scanner against
 * scan_motif_word64_shifted_count on 4x Tesla V100-SXM2-16GB, sm_70.
 * Commands:
 *   ./build/baseplaneDna2Bench 67108864 16 1 30 packed_word64 20260504 single_gpu
 *   ./build/baseplaneDna2Bench 67108864 16 1 30 packed_word64_shifted_count 20260504 single_gpu
 *   ./build/baseplaneDna2Bench 1073741824 16 1 20 packed_word64_shifted_count 20260504 all_gpus
 * Results: old packed reference 3.155 ms / 21.3G windows/s on one V100;
 * shifted count 0.222 ms / 301.9G windows/s on one V100 for 67M bases;
 * 1.073B bases reached 3.644 ms / 294.7G windows/s on one V100 and
 * 0.952 ms / 1.128T windows/s across four V100s. Correctness was validated
 * against CPU references by baseplaneDna2CudaTest.
 *
 * Plane-stream CUDA benchmark note, 2026-06-17:
 * Compared allocation-free resident stream primitives against CPU references
 * in baseplaneDna2Test/baseplaneDna2CudaTest on 4x Tesla V100-SXM2-16GB,
 * sm_70, CUDA 12.9.86, CMAKE_CUDA_ARCHITECTURES=70.
 * Commands:
 *   ./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_convert 20260617 single_gpu
 *   ./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_base_mask 20260617 single_gpu
 *   ./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_gc_mask 20260617 single_gpu
 *   ./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_cpg_start_mask 20260617 single_gpu
 * Results: convert 0.045 ms for 67.1M bases / 2.1M words; base mask 0.034 ms;
 * GC mask 0.034 ms; CpG-start mask 0.034 ms. Kernels used 16-18 registers per
 * thread, 0 local bytes, 0 static shared bytes. Outputs were one uint32_t mask
 * per 32-base word; CpG-start includes adjacent-word lane-31 starts. Numerical
 * behavior matched CPU references with final-word masks applied by tests.
 */

#include <Baseplane/seq/dna2.cuh>

#include <climits>

namespace baseplane::seq {

namespace {

__device__ __forceinline__ std::uint64_t required_word_count_device(int n_bases) {
    return n_bases <= 0 ? 0ULL : (static_cast<std::uint64_t>(n_bases) + 31ULL) >> 5u;
}

__device__ __forceinline__ std::uint32_t compact_even_bits_device(std::uint64_t bits) {
    bits &= 0x5555555555555555ULL;
    bits = (bits | (bits >> 1u)) & 0x3333333333333333ULL;
    bits = (bits | (bits >> 2u)) & 0x0f0f0f0f0f0f0f0fULL;
    bits = (bits | (bits >> 4u)) & 0x00ff00ff00ff00ffULL;
    bits = (bits | (bits >> 8u)) & 0x0000ffff0000ffffULL;
    bits = (bits | (bits >> 16u)) & 0x00000000ffffffffULL;
    return static_cast<std::uint32_t>(bits);
}

__device__ __forceinline__ std::uint32_t base_mask_from_code_device(
    std::uint32_t lo,
    std::uint32_t hi,
    std::uint8_t code) {
    return code == 0u ? (~lo & ~hi)
        : code == 1u ? (lo & ~hi)
        : code == 2u ? (~lo & hi)
        : (lo & hi);
}

__device__ __forceinline__ std::uint64_t shifted_window_word64(
    const std::uint64_t* packed_seq,
    std::uint64_t n_words,
    int start) {
    const unsigned int word_index = static_cast<unsigned int>(start) >> 5u;
    const unsigned int shift = (static_cast<unsigned int>(start) & 31u) * 2u;
    const std::uint64_t lo = packed_seq[word_index];
    if (shift == 0u) return lo;
    const std::uint64_t hi = (static_cast<std::uint64_t>(word_index) + 1ULL) < n_words
        ? packed_seq[word_index + 1u]
        : 0ULL;
    return (lo >> shift) | (hi << (64u - shift));
}

int scan_blocks_for_windows(std::uint64_t windows, int threads) {
    const std::uint64_t blocks = (windows + static_cast<std::uint64_t>(threads) - 1ULL)
        / static_cast<std::uint64_t>(threads);
    return static_cast<int>(blocks > 8192ULL ? 8192ULL : blocks);
}

int blocks_for_words(std::uint64_t n_words, int threads) {
    const std::uint64_t blocks = (n_words + static_cast<std::uint64_t>(threads) - 1ULL)
        / static_cast<std::uint64_t>(threads);
    return static_cast<int>(blocks > 8192ULL ? 8192ULL : blocks);
}

bool valid_cuda_scan_input(dna2_packed64_view sequence, motif32_exact motif) {
    if (motif.length == 0u || motif.length > 32u) return false;
    if (sequence.n_bases > static_cast<std::uint64_t>(INT_MAX)) return false;
    if (sequence.n_bases > 0u && sequence.words == nullptr) return false;
    const std::uint64_t required_words = (sequence.n_bases + 31ULL) >> 5u;
    if (sequence.n_words < required_words) return false;
    return true;
}

bool valid_cuda_packed_stream_input(dna2_packed64_view sequence) {
    if (sequence.n_bases > static_cast<std::uint64_t>(INT_MAX)) return false;
    if (sequence.n_bases > 0u && sequence.words == nullptr) return false;
    const std::uint64_t required_words = (sequence.n_bases + 31ULL) >> 5u;
    if (sequence.n_words < required_words) return false;
    return true;
}

bool valid_cuda_planes_stream_output(dna2_planes32_stream_mutable_view output, std::uint64_t n_words) {
    if (output.n_words < n_words) return false;
    if (n_words > 0u && (output.lo_words == nullptr || output.hi_words == nullptr)) return false;
    return true;
}

bool valid_cuda_planes_stream_input(dna2_planes32_stream_view input) {
    if (input.n_words > 0u && (input.lo_words == nullptr || input.hi_words == nullptr)) return false;
    return true;
}

bool valid_cuda_mask_stream_output(dna2_mask32_stream_mutable_view output, std::uint64_t n_words) {
    if (output.n_words < n_words) return false;
    if (n_words > 0u && output.masks == nullptr) return false;
    return true;
}

} // namespace

__global__ void scan_motif_warp32_unpacked(
    const std::uint8_t* seq_bases,
    int n_bases,
    dna2_planes32 motif,
    int motif_len,
    int max_mismatches,
    std::uint8_t* hits) {
    const int global_thread = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int lane = static_cast<int>(threadIdx.x & 31u);
    const int first_start = global_thread >> 5;
    const int warp_stride = static_cast<int>((gridDim.x * blockDim.x) >> 5u);
    const int windows = n_bases >= motif_len ? n_bases - motif_len + 1 : 0;
    if (warp_stride <= 0) return;

    for (int start = first_start; start < windows; start += warp_stride) {
        std::uint8_t base = 0u;
        if (lane < motif_len) {
            base = seq_bases[start + lane] & 0x3u;
        }

        const dna2_planes32 window = warp_encode_base_lanes(base);
        if (lane == 0) {
            const std::uint32_t active_mask = detail::active_mask_from_length(motif_len);
            const int mismatches = planes32_mismatches(window, motif, active_mask);
            hits[start] = static_cast<std::uint8_t>(mismatches <= max_mismatches ? 1u : 0u);
        }
    }
}

__global__ void convert_word64_to_planes32_warp(
    const std::uint64_t* packed_words,
    dna2_planes32* planes,
    int word_count) {
    if (packed_words == nullptr || planes == nullptr || word_count <= 0 || (blockDim.x & 31u) != 0u) return;

    const int lane = static_cast<int>(threadIdx.x & 31u);
    const int warp_in_block = static_cast<int>(threadIdx.x >> 5u);
    const int warps_per_block = static_cast<int>(blockDim.x >> 5u);
    const int word_index = static_cast<int>(blockIdx.x) * warps_per_block + warp_in_block;
    if (word_index >= word_count) return;

    const std::uint64_t packed = packed_words[word_index];
    const std::uint8_t base = static_cast<std::uint8_t>((packed >> (2u * static_cast<unsigned int>(lane))) & 0x3ULL);
    const dna2_planes32 word_planes = warp_encode_base_lanes(base);
    if (lane == 0) {
        planes[word_index] = word_planes;
    }
}

__global__ void scan_motif_word64_reference(
    const std::uint64_t* packed_seq,
    int n_bases,
    dna2_word64 motif_word,
    int motif_len,
    int max_mismatches,
    std::uint8_t* hits) {
    const int start = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int windows = n_bases >= motif_len ? n_bases - motif_len + 1 : 0;
    if (start >= windows) return;

    dna2_word64 window{0ULL};
    for (int i = 0; i < motif_len; ++i) {
        const int seq_index = start + i;
        const dna2_word64 source{packed_seq[static_cast<unsigned int>(seq_index) >> 5u]};
        set_base(window, i, get_base(source, seq_index & 31));
    }

    const std::uint32_t active_mask = detail::active_mask_from_length(motif_len);
    const int mismatches = word64_mismatches(window, motif_word, active_mask);
    hits[start] = static_cast<std::uint8_t>(mismatches <= max_mismatches ? 1u : 0u);
}

__global__ void scan_motif_word64_shifted_count(
    const std::uint64_t* packed_seq,
    int n_bases,
    dna2_word64 motif_word,
    int motif_len,
    int max_mismatches,
    unsigned long long* hit_count) {
    extern __shared__ unsigned int block_counts[];
    const int tid = static_cast<int>(threadIdx.x);
    const int global_thread = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int stride = static_cast<int>(gridDim.x * blockDim.x);
    const int windows = n_bases >= motif_len ? n_bases - motif_len + 1 : 0;
    const std::uint32_t active_mask = detail::active_mask_from_length(motif_len);
    const std::uint64_t active_fields = detail::spread_active_mask_to_packed_fields(active_mask);
    unsigned int local_hits = 0u;

    for (int start = global_thread; start < windows; start += stride) {
        const dna2_word64 window{shifted_window_word64(packed_seq, required_word_count_device(n_bases), start)};
        const int mismatches = detail::word64_mismatches_packed_count_fields(window, motif_word, active_fields);
        local_hits += mismatches <= max_mismatches ? 1u : 0u;
    }

    block_counts[tid] = local_hits;
    __syncthreads();
    for (int offset = static_cast<int>(blockDim.x) >> 1; offset > 0; offset >>= 1) {
        if (tid < offset) block_counts[tid] += block_counts[tid + offset];
        __syncthreads();
    }
    if (tid == 0) {
        atomicAdd(hit_count, static_cast<unsigned long long>(block_counts[0]));
    }
}

__global__ void scan_motif_word64_shifted_emit(
    const std::uint64_t* packed_seq,
    int n_bases,
    motif32_exact motif,
    compact_motif_hit_buffer hits) {
    const int global_thread = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int stride = static_cast<int>(gridDim.x * blockDim.x);
    const int windows = n_bases >= motif.length ? n_bases - static_cast<int>(motif.length) + 1 : 0;
    const dna2_word64 motif_word{motif.packed};
    const std::uint64_t n_words = required_word_count_device(n_bases);

    for (int start = global_thread; start < windows; start += stride) {
        const dna2_word64 window{shifted_window_word64(packed_seq, n_words, start)};
        const int mismatches = detail::word64_mismatches_packed_count_fields(window, motif_word, motif.active_fields);
        if (mismatches > motif.max_mismatches) continue;

        const std::uint32_t slot = atomicAdd(hits.records_written, 1u);
        if (slot < hits.capacity) {
            hits.hits[slot] = motif_hit{
                static_cast<std::uint32_t>(start),
                motif.motif_id,
                static_cast<std::uint8_t>(mismatches),
                0u
            };
        } else {
            atomicAdd(hits.records_dropped, 1u);
        }
    }
}

__global__ void scan_motif_word64_aligned_count(
    const std::uint64_t* packed_words,
    int word_count,
    dna2_word64 motif_word,
    int motif_len,
    int max_mismatches,
    unsigned long long* hit_count) {
    extern __shared__ unsigned int block_counts[];
    const int tid = static_cast<int>(threadIdx.x);
    const int global_thread = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int stride = static_cast<int>(gridDim.x * blockDim.x);
    const std::uint32_t active_mask = detail::active_mask_from_length(motif_len);
    const std::uint64_t active_fields = detail::spread_active_mask_to_packed_fields(active_mask);
    unsigned int local_hits = 0u;

    for (int base_index = global_thread * 4; base_index < word_count; base_index += stride * 4) {
        #pragma unroll
        for (int lane = 0; lane < 4; ++lane) {
            const int word_index = base_index + lane;
            if (word_index < word_count) {
                const dna2_word64 window{packed_words[word_index]};
                const int mismatches = detail::word64_mismatches_packed_count_fields(window, motif_word, active_fields);
                local_hits += mismatches <= max_mismatches ? 1u : 0u;
            }
        }
    }

    block_counts[tid] = local_hits;
    __syncthreads();
    for (int offset = static_cast<int>(blockDim.x) >> 1; offset > 0; offset >>= 1) {
        if (tid < offset) block_counts[tid] += block_counts[tid + offset];
        __syncthreads();
    }
    if (tid == 0) {
        atomicAdd(hit_count, static_cast<unsigned long long>(block_counts[0]));
    }
}

__global__ void scan_motif_inlplane64_aligned_count(
    const std::uint64_t* inlplanes,
    int word_count,
    dna2_inlplane64 motif,
    int motif_len,
    int max_mismatches,
    unsigned long long* hit_count) {
    extern __shared__ unsigned int block_counts[];
    const int tid = static_cast<int>(threadIdx.x);
    const int global_thread = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    const int stride = static_cast<int>(gridDim.x * blockDim.x);
    const std::uint32_t active_mask = detail::active_mask_from_length(motif_len);
    unsigned int local_hits = 0u;

    for (int base_index = global_thread * 4; base_index < word_count; base_index += stride * 4) {
        #pragma unroll
        for (int lane = 0; lane < 4; ++lane) {
            const int word_index = base_index + lane;
            if (word_index < word_count) {
                const dna2_inlplane64 window{inlplanes[word_index]};
                const int mismatches = inlplane64_mismatches(window, motif, active_mask);
                local_hits += mismatches <= max_mismatches ? 1u : 0u;
            }
        }
    }

    block_counts[tid] = local_hits;
    __syncthreads();
    for (int offset = static_cast<int>(blockDim.x) >> 1; offset > 0; offset >>= 1) {
        if (tid < offset) block_counts[tid] += block_counts[tid + offset];
        __syncthreads();
    }
    if (tid == 0) {
        atomicAdd(hit_count, static_cast<unsigned long long>(block_counts[0]));
    }
}

__global__ void dna2_to_planes32_stream_kernel(
    const std::uint64_t* packed_words,
    std::uint32_t* lo_words,
    std::uint32_t* hi_words,
    std::uint64_t n_words) {
    const std::uint64_t stride = static_cast<std::uint64_t>(gridDim.x) * static_cast<std::uint64_t>(blockDim.x);
    for (std::uint64_t word = static_cast<std::uint64_t>(blockIdx.x) * static_cast<std::uint64_t>(blockDim.x)
             + static_cast<std::uint64_t>(threadIdx.x);
         word < n_words;
         word += stride) {
        const std::uint64_t packed = packed_words[word];
        lo_words[word] = compact_even_bits_device(packed);
        hi_words[word] = compact_even_bits_device(packed >> 1u);
    }
}

__global__ void planes32_stream_base_mask_kernel(
    const std::uint32_t* lo_words,
    const std::uint32_t* hi_words,
    std::uint8_t base_code,
    std::uint32_t* output_masks,
    std::uint64_t n_words) {
    const std::uint64_t stride = static_cast<std::uint64_t>(gridDim.x) * static_cast<std::uint64_t>(blockDim.x);
    for (std::uint64_t word = static_cast<std::uint64_t>(blockIdx.x) * static_cast<std::uint64_t>(blockDim.x)
             + static_cast<std::uint64_t>(threadIdx.x);
         word < n_words;
         word += stride) {
        output_masks[word] = base_mask_from_code_device(lo_words[word], hi_words[word], base_code);
    }
}

__global__ void planes32_stream_gc_mask_kernel(
    const std::uint32_t* lo_words,
    const std::uint32_t* hi_words,
    std::uint32_t* output_masks,
    std::uint64_t n_words) {
    const std::uint64_t stride = static_cast<std::uint64_t>(gridDim.x) * static_cast<std::uint64_t>(blockDim.x);
    for (std::uint64_t word = static_cast<std::uint64_t>(blockIdx.x) * static_cast<std::uint64_t>(blockDim.x)
             + static_cast<std::uint64_t>(threadIdx.x);
         word < n_words;
         word += stride) {
        output_masks[word] = lo_words[word] ^ hi_words[word];
    }
}

__global__ void planes32_stream_cpg_start_mask_kernel(
    const std::uint32_t* lo_words,
    const std::uint32_t* hi_words,
    std::uint32_t* output_masks,
    std::uint64_t n_words) {
    const std::uint64_t stride = static_cast<std::uint64_t>(gridDim.x) * static_cast<std::uint64_t>(blockDim.x);
    for (std::uint64_t word = static_cast<std::uint64_t>(blockIdx.x) * static_cast<std::uint64_t>(blockDim.x)
             + static_cast<std::uint64_t>(threadIdx.x);
         word < n_words;
         word += stride) {
        const std::uint32_t lo = lo_words[word];
        const std::uint32_t hi = hi_words[word];
        const std::uint32_t c_mask = base_mask_from_code_device(lo, hi, 1u);
        const std::uint32_t g_mask = base_mask_from_code_device(lo, hi, 2u);
        std::uint32_t mask = c_mask & (g_mask >> 1u) & 0x7fffffffu;
        if ((word + 1u) < n_words) {
            const std::uint32_t next_g_bit0 =
                base_mask_from_code_device(lo_words[word + 1u], hi_words[word + 1u], 2u) & 0x00000001u;
            if ((c_mask & 0x80000000u) != 0u && next_g_bit0 != 0u) mask |= 0x80000000u;
        }
        output_masks[word] = mask;
    }
}

baseplane::status scan_exact_count_cuda(
    cudaStream_t stream,
    dna2_packed64_view sequence,
    motif32_exact motif,
    unsigned long long* device_count) {
    if (device_count == nullptr || !valid_cuda_scan_input(sequence, motif)) {
        return baseplane::invalid_argument_status();
    }

    cudaError_t err = cudaMemsetAsync(device_count, 0, sizeof(unsigned long long), stream);
    if (err != cudaSuccess) return baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
    if (sequence.n_bases < motif.length) {
        return baseplane::ok_status();
    }

    const std::uint64_t windows = sequence.n_bases - motif.length + 1ULL;
    constexpr int threads = 256;
    const int blocks = scan_blocks_for_windows(windows, threads);
    scan_motif_word64_shifted_count<<<blocks, threads, threads * sizeof(unsigned int), stream>>>(
        sequence.words,
        static_cast<int>(sequence.n_bases),
        motif_word(motif),
        static_cast<int>(motif.length),
        static_cast<int>(motif.max_mismatches),
        device_count);
    err = cudaGetLastError();
    return err == cudaSuccess
        ? baseplane::ok_status()
        : baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
}

baseplane::status dna2_to_planes32_stream_cuda(
    cudaStream_t stream,
    dna2_packed64_view sequence,
    dna2_planes32_stream_mutable_view output) {
    if (!valid_cuda_packed_stream_input(sequence)) {
        return baseplane::invalid_argument_status();
    }
    const std::uint64_t n_words = (sequence.n_bases + 31ULL) >> 5u;
    if (!valid_cuda_planes_stream_output(output, n_words)) {
        return baseplane::invalid_argument_status();
    }
    if (n_words == 0u) return baseplane::ok_status();

    constexpr int threads = 256;
    const int blocks = blocks_for_words(n_words, threads);
    dna2_to_planes32_stream_kernel<<<blocks, threads, 0, stream>>>(
        sequence.words,
        output.lo_words,
        output.hi_words,
        n_words);
    const cudaError_t err = cudaGetLastError();
    return err == cudaSuccess
        ? baseplane::ok_status()
        : baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
}

baseplane::status planes32_stream_base_mask_cuda(
    cudaStream_t stream,
    dna2_planes32_stream_view input,
    std::uint8_t base_code,
    dna2_mask32_stream_mutable_view output) {
    if (base_code > 3u || !valid_cuda_planes_stream_input(input)
        || !valid_cuda_mask_stream_output(output, input.n_words)) {
        return baseplane::invalid_argument_status();
    }
    if (input.n_words == 0u) return baseplane::ok_status();

    constexpr int threads = 256;
    const int blocks = blocks_for_words(input.n_words, threads);
    planes32_stream_base_mask_kernel<<<blocks, threads, 0, stream>>>(
        input.lo_words,
        input.hi_words,
        base_code,
        output.masks,
        input.n_words);
    const cudaError_t err = cudaGetLastError();
    return err == cudaSuccess
        ? baseplane::ok_status()
        : baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
}

baseplane::status planes32_stream_gc_mask_cuda(
    cudaStream_t stream,
    dna2_planes32_stream_view input,
    dna2_mask32_stream_mutable_view output) {
    if (!valid_cuda_planes_stream_input(input) || !valid_cuda_mask_stream_output(output, input.n_words)) {
        return baseplane::invalid_argument_status();
    }
    if (input.n_words == 0u) return baseplane::ok_status();

    constexpr int threads = 256;
    const int blocks = blocks_for_words(input.n_words, threads);
    planes32_stream_gc_mask_kernel<<<blocks, threads, 0, stream>>>(
        input.lo_words,
        input.hi_words,
        output.masks,
        input.n_words);
    const cudaError_t err = cudaGetLastError();
    return err == cudaSuccess
        ? baseplane::ok_status()
        : baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
}

baseplane::status planes32_stream_cpg_start_mask_cuda(
    cudaStream_t stream,
    dna2_planes32_stream_view input,
    dna2_mask32_stream_mutable_view output) {
    if (!valid_cuda_planes_stream_input(input) || !valid_cuda_mask_stream_output(output, input.n_words)) {
        return baseplane::invalid_argument_status();
    }
    if (input.n_words == 0u) return baseplane::ok_status();

    constexpr int threads = 256;
    const int blocks = blocks_for_words(input.n_words, threads);
    planes32_stream_cpg_start_mask_kernel<<<blocks, threads, 0, stream>>>(
        input.lo_words,
        input.hi_words,
        output.masks,
        input.n_words);
    const cudaError_t err = cudaGetLastError();
    return err == cudaSuccess
        ? baseplane::ok_status()
        : baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
}

baseplane::status scan_exact_emit_cuda(
    cudaStream_t stream,
    dna2_packed64_view sequence,
    motif32_exact motif,
    compact_motif_hit_buffer device_hits) {
    if (device_hits.records_written == nullptr || device_hits.records_dropped == nullptr
        || (device_hits.capacity > 0u && device_hits.hits == nullptr)
        || !valid_cuda_scan_input(sequence, motif)) {
        return baseplane::invalid_argument_status();
    }

    cudaError_t err = cudaMemsetAsync(device_hits.records_written, 0, sizeof(std::uint32_t), stream);
    if (err != cudaSuccess) return baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
    err = cudaMemsetAsync(device_hits.records_dropped, 0, sizeof(std::uint32_t), stream);
    if (err != cudaSuccess) return baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
    if (sequence.n_bases < motif.length) {
        return baseplane::ok_status();
    }

    const std::uint64_t windows = sequence.n_bases - motif.length + 1ULL;
    constexpr int threads = 256;
    const int blocks = scan_blocks_for_windows(windows, threads);
    scan_motif_word64_shifted_emit<<<blocks, threads, 0, stream>>>(
        sequence.words,
        static_cast<int>(sequence.n_bases),
        motif,
        device_hits);
    err = cudaGetLastError();
    return err == cudaSuccess
        ? baseplane::ok_status()
        : baseplane::cuda_error_status(static_cast<std::uint32_t>(err));
}

} // namespace baseplane::seq
