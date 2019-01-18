# Command-Line Arguments

## Run

> $ mkdir build && cd build && cmake .. && make

## Usage

* `$ ./args a b c`
* `$ ./args "a b c"` : `""` 기호를 사용하면 shell에서 하나의 인자로 묶어준다.
* `$ ./args *.c` : `*`, `?`와 같은 wild card를 사용하면 shell이 glob pattern으로 확장시켜준다.
