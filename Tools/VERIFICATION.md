# Reproduce binary comparisons

The Actions workflow automatically runs these comparisons and saves their JSON reports with the compiler evidence. Both scripts require the exact supplied original executable, checked by SHA-256.

On Linux:

```sh
python3 -m venv build/verification-venv
build/verification-venv/bin/pip install -r Tools/requirements-verification.txt
c++ -std=c++17 -O2 -shared -fPIC -ICore Core/Geometry.cpp Tools/GeometryBridge.cpp -o build/geometry-bridge.so
c++ -std=c++17 -O2 -shared -fPIC -ICore Core/CoordinateAccess.cpp Tools/CoordinateBridge.cpp -o build/coordinate-bridge.so
build/verification-venv/bin/python Tools/compare_geometry.py --binary /path/to/original/TrollSpeed --library build/geometry-bridge.so --output build/geometry-binary-comparison.json
build/verification-venv/bin/python Tools/compare_coordinate_vm.py --binary /path/to/original/TrollSpeed --library build/coordinate-bridge.so --output build/coordinate-binary-comparison.json
```

On macOS use `-dynamiclib` instead of `-shared -fPIC` and use `.dylib` file names.

The geometry script compares 800 finite-input cases. The VM script compares 22 candidate-validation/patching cases, including memory faults. The controlled boundaries and evidence scope are recorded in each report. Neither comparison establishes actual game compatibility or produces a device screenshot.
