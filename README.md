# utf8 validation experiment

To implement and see how the ["Shift based DFA"][1] works out in a "practical setting".

[1]: https://gist.github.com/pervognsen/218ea17743e1442e59bb60d29b1aa725


## results

```
file: ./validateutf8-experiments/examples/twitter.json
naive          : final state=ok, time= 2583175, bytes/time=0.244472
flipped        : final state=ok, time= 1974415, bytes/time=0.319849
shift1         : final state=ok, time=  946216, bytes/time=0.667411
shift3         : final state=ok, time=  397088, bytes/time=1.590365
shift3_unroll  : final state=ok, time=  322068, bytes/time=1.960813

file: ./validateutf8-experiments/examples/hongkong.html
naive          : final state=ok, time= 7109834, bytes/time=0.254177
flipped        : final state=ok, time= 5335529, bytes/time=0.338702
shift1         : final state=ok, time= 2713792, bytes/time=0.665915
shift3         : final state=ok, time= 1113694, bytes/time=1.622668
shift3_unroll  : final state=ok, time=  903289, bytes/time=2.000640
```
