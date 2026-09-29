"""Packaging/guard checks; these do not execute or validate any CUDA kernel."""
from __future__ import annotations
import copy, importlib.util, json, os, subprocess, sys, tempfile, unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import check
import install
import run

class PackageTests(unittest.TestCase):
    def test_package_contract(self):
        self.assertEqual(check.check_package(ROOT)['native_task_count'],7)
    def test_template_starts_unrun(self):
        value=check.read_json(ROOT/'results/TEMPLATE.json')
        self.assertEqual(value['status'],'not_run')
        self.assertFalse(value['cuda']['compiled'])
        self.assertFalse(value['cuda']['executed'])
    def test_template_cannot_pass(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'results').mkdir()
            (root/'results'/'bitlift.json').write_text((ROOT/'results/TEMPLATE.json').read_text())
            with self.assertRaises(ValueError):check.check_result(root,'bitlift')
    def test_evidence_cannot_escape(self):
        with self.assertRaises(ValueError):check.evidence_paths(ROOT,['../elsewhere'],'test')
        with self.assertRaises(ValueError):check.evidence_paths(ROOT,['/etc/passwd'],'test')
    def test_nonempty_evidence_required(self):
        with self.assertRaises(ValueError):check.evidence_paths(ROOT,[],'test')
    def test_negative_selection_needs_no_fake_model(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'results').mkdir();(root/'results/report.md').write_text('Synthetic checker fixture; not an experiment result.')
            value={'status':'completed','selected':[],'rationale':'No candidate supported in this synthetic checker fixture.','evidence':['results/report.md']}
            (root/'results/selection.json').write_text(json.dumps(value))
            self.assertEqual(check.check_selection(root,[])['selected'],[])
    def test_positive_selection_needs_execution_evidence(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'results').mkdir();(root/'results/report.md').write_text('Synthetic checker fixture.')
            value={'status':'completed','selected':['bitlift'],'rationale':'synthetic fixture','evidence':['results/report.md']}
            (root/'results/selection.json').write_text(json.dumps(value))
            with self.assertRaises(ValueError):check.check_selection(root,[{'experiment':'bitlift','verdict':'promote_candidate'}])
    def test_install_preview_does_not_write(self):
        with tempfile.TemporaryDirectory() as d:
            root=self.repo(Path(d));result=install.install(root,False)
            self.assertEqual(result['status'],'preview');self.assertFalse((root/'experiments').exists())
    def test_install_additive_idempotent(self):
        with tempfile.TemporaryDirectory() as d:
            root=self.repo(Path(d));before=(root/'CMakeLists.txt').read_bytes()
            result=install.install(root,True)
            self.assertTrue(result['applied']);self.assertEqual((root/'CMakeLists.txt').read_bytes(),before)
            self.assertEqual(install.install(root,True)['status'],'already_identical')
    def test_install_conflict_preserves_files(self):
        with tempfile.TemporaryDirectory() as d:
            root=self.repo(Path(d));target=root/'experiments/cuda_lab';target.mkdir(parents=True);(target/'keep.txt').write_text('keep')
            with self.assertRaises(ValueError):install.install(root,True)
            self.assertEqual(list(target.iterdir()),[target/'keep.txt']);self.assertEqual((target/'keep.txt').read_text(),'keep')
    def test_install_rejects_symlink(self):
        with tempfile.TemporaryDirectory() as d:
            root=self.repo(Path(d));other=root/'other';other.mkdir();(root/'experiments').symlink_to(other,target_is_directory=True)
            with self.assertRaises(ValueError):install.install(root,True)
    def test_no_cuda_is_unavailable_not_pass(self):
        with tempfile.TemporaryDirectory() as d, patch.dict(os.environ,{},clear=True), patch('run.shutil.which',return_value=None):
            self.assertEqual(run.main(['--phase','build-cuda','--build-dir',str(Path(d)/'build')]),77)
    def test_gpu_requires_assignment(self):
        with tempfile.TemporaryDirectory() as d, patch.dict(os.environ,{},clear=True):
            self.assertEqual(run.main(['--phase','gpu','--build-dir',str(Path(d)/'build')]),1)
    def test_reject_unrelated_build(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'keep').write_text('unrelated')
            self.assertEqual(run.main(['--phase','host','--build-dir',str(root)]),1)
            self.assertEqual((root/'keep').read_text(),'unrelated')
    def test_receipt_does_not_overwrite_package_sources(self):
        with tempfile.TemporaryDirectory() as d:
            self.assertEqual(run.main(['--phase','host','--build-dir',str(Path(d)/'build'),'--output',str(ROOT/'planning/epic.v2.json')]),1)
    @staticmethod
    def repo(path):
        (path/'CMakeLists.txt').write_text('project(Baseplane LANGUAGES CXX)\n');(path/'AGENTS.md').write_text('Unrelated original guidance.\n');return path
if __name__=='__main__':unittest.main()
