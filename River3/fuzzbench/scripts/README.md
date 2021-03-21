In order to generate the binaries:
- install fuzzbench (you will need ~100GB for storage):
    ` ./install_fuzzbench.sh `
- patch commit b2d2897 (this adds an empty fuzzer):
    ` (cd fuzzbench && git checkout b2d2897 && git apply patch_b2d2897) `
- modify compile commands(CC, CFLAGS) and binaries path in the script and run it:
    ` ./get_executables.sh `
- modify `get_text` script to add new commands for each binary:
    ` ./get_text /path/to/tar/archive `

Binaries (compiled normally and with -static flag) + text info can be found at: 
https://drive.google.com/file/d/1OR2x2JAGJhmWxp-sUqPHOmA58R4Tmp1s/view?usp=sharing 
