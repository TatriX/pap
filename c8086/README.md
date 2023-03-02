# Usage

```sh
make translate
make
./c8086 ./translated/listing-37
./c8086 ./translated/listing-38

./c8086 ./translated/listing-37 | diff listing-37.asm -
./c8086 ./translated/listing-38 | diff listing-38.asm -
```
