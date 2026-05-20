# Demos of raw usages

## `alpha.c`: alpha channel

Illustrates alpha channel support by rendering a PNG with the following shapes:

- **Bottom half**: A red opaque rectangle with three overlapping triangles at different alpha levels — green (α=0.8, nearly opaque), blue (α=0.5, half-transparent showing the red through), and cyan (α=0.25, mostly transparent)
- **Upper left**: A smooth-shaded magenta triangle that fades from fully opaque at the base to fully transparent at the tip — demonstrating per-vertex alpha interpolation
- **Upper right**: A checkerboard RGBA texture where the white squares fade from transparent (bottom) to opaque (top), and the golden squares have α=200/255 — demonstrating per-texel texture alpha

To run this demo:

```sh
cc -std=c99 -O2 -Iinclude -Iexamples examples/raw/alpha.c lib/libTinyGL.a -lm -o alpha && ./alpha
```

Then find `alpha_demo.png` in your working directory. Expected image:

<img width="512" height="512" alt="alpha_demo" src="https://github.com/user-attachments/assets/fa3ae5c0-961b-40a5-899d-6684a1403a94" />
