# WaveFunctionCollapse
My [WaveFunctionCollapse](https://github.com/mxgmn/WaveFunctionCollapse) implementation in C++. Currently only the SimpleTiledModel.

## Installation
Extract the zip file anywhere.

## Running
The executable takes 3 arguments:
1. Path to example map file _(assets/example_map/testmap.csv)_
2. Path to output rng maps _(assets/output_maps)_
3. Path to colormap file _(assets/example_map/testmap_colors.txt)_

PowerShell:
```
.\WFC.exe .\assets\example_map\testmap.csv .\assets\output_maps .\assets\example_map\testmap_colors.txt
```

## Extras
- _assets/example_map/testmap.json_ - is a [Tiled](https://www.mapeditor.org/) editable map
- _assets/example_map/colors.png_ - Used to build test map in Tiled.
