# Quadtree Compression
A simple program that uses quadtrees to compress, decompress and mirror images.

![](quadtree.png)

# Build
Use the included Makefile
```
make
```

# Usage

```
./quadtree -m <flip_mode> <compression_level> <input_image> <output_image>
```
## Compressing an image
```
./quadtree -c 200 input_image.ppm binary.out
```
<img src="https://i.imgur.com/LjjRbYo.png" width="250"> 

## Decompress an image

```
./quadtree -d <binary_input> <output_image>
```

## Flip an image

```
./quadtree -m <mode> 0 <input_image> <output_image>
```
### Horizontal
```
./quadtree -m h 0 image.ppm horizontal.ppm
```
<img src="https://i.imgur.com/bvaIPCy.png" width="250">

### Vertical
```
./quadtree -m v 0 image.ppm vertical.ppm
```
<img src="https://i.imgur.com/x87d0ZO.png" width="250">

## Completion Date
2018, April 20
