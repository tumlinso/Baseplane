#!/usr/bin/env python3
"""Check the additive adoption inventory against the preserved catalogues."""
import json
from pathlib import Path

root = Path(__file__).resolve().parents[3]
here = Path(__file__).resolve().parent
load = lambda path: json.loads(path.read_text())
actual = load(here / "source-dispositions.json")
catalogue = load(root / "planning/baseplane_moonshot_bootstrap/results/catalogue.json")
seed = load(root / "planning/integrated-substrate-v1/machine/historical-bitop-map.json")
assert {x["id"] for x in actual["cards"]} == {"BP:" + x["id"] for x in catalogue["cards"]}
assert len(actual["cards"]) == 48
assert {x["old_id"] for x in actual["historical_bitop"]} == {x["historical_id"] for x in seed["rows"]}
assert len(actual["historical_bitop"]) == 33
for card in actual["cards"]:
    assert card["sequence_consumer_owner"] and card["general_math_owner"]
    assert card["remaining_variants"] and not card["biological_validation"]
    for path in card["source"] + card["evidence"]:
        assert (root / path).exists(), path
for row in actual["historical_bitop"]:
    assert row["integrated_substrate_owner"]
    assert not row["old_production_task_claimed_complete"]
for path in actual["historical_status_authority"]:
    assert (root / path).exists(), path
print("48 cards, 33 historical obligations, explicit owners and preserved sources checked")
