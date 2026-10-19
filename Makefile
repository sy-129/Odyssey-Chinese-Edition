CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Werror -pedantic -O2
.PHONY: check clean
check:
	mkdir -p build
	$(CXX) $(CXXFLAGS) -ICore Core/RemoteMemory.cpp Core/Geometry.cpp Tests/MemoryTests.cpp -o build/memory-tests
	./build/memory-tests
	$(CXX) $(CXXFLAGS) -ICore -c Core/GameData.cpp -o build/GameData.o
	$(CXX) $(CXXFLAGS) -ICore Core/CoordinateAccess.cpp Tests/CoordinateTests.cpp -o build/coordinate-tests
	./build/coordinate-tests
	$(CXX) $(CXXFLAGS) -ICore Core/RemoteMemory.cpp Core/Geometry.cpp Core/GameData.cpp Tests/GameDataTests.cpp -o build/game-data-tests
	./build/game-data-tests
	$(CXX) $(CXXFLAGS) -ICore -IVendor/imgui Core/Geometry.cpp Core/ESPRenderer.cpp Tests/RendererTests.cpp Vendor/imgui/imgui.cpp Vendor/imgui/imgui_draw.cpp Vendor/imgui/imgui_tables.cpp Vendor/imgui/imgui_widgets.cpp -o build/renderer-tests
	./build/renderer-tests
	$(CXX) $(CXXFLAGS) -ICore -IVendor/imgui Core/SettingsMenu.cpp Tests/MenuTests.cpp Vendor/imgui/imgui.cpp Vendor/imgui/imgui_draw.cpp Vendor/imgui/imgui_tables.cpp Vendor/imgui/imgui_widgets.cpp -o build/menu-tests
	./build/menu-tests
clean:
	rm -rf build
