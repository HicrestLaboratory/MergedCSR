## Instructions
The `g++` compiler supporting C++11 or later is required to build the project. The project can be built by running `make` in the project directory. The `make` targets are listed in the table below.

## Datasets
| Make target | Dataset           | `\|V\|` | `\|E\|` | Notes          | Source    |
|-------------|-------------------|---------|---------|----------------|-----------|
| test        | test              | 11      | 25      | Handcrafted    |           |
| random      | random_1k_5k      | 1000    | 9940    | Small diameter | Speedcode |
| facebook    | facebook_combined | 4039    | 88234   | Small diameter | [^1]      |


[^1]: [https://snap.stanford.edu/data/ego-Facebook.html](https://snap.stanford.edu/data/ego-Facebook.html).