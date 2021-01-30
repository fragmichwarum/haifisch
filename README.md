# Haifisch

## Convenient matrix library

### Run tests:
```
cd test
cmake .
make
./haifisch_test
```

### Performance test:
```
 CPU: AMD Ryzen 5 2500U.

  [int]    [500x500]   [8 threads] haifisch mul       -> 0.05766 s.
  [int]    [500x500]   [1 thread ] boost ublas mul    -> 3.25886 s.

  [int]    [1000x1000] [8 threads] haifisch mul       -> 0.66614 s.
  [int]    [1000x1000] [1 thread ] boost ublas mul    -> 25.6260 s.

  [int]    [1500x1500] [8 threads] haifisch mul       -> 3.05941 s.
  [int]    [1500x1500] [1 thread ] boost ublas mul    -> 79.8228 s.

  [float]  [500x500]   [8 threads] haifisch mul       -> 0.05239 s.
  [float]  [500x500]   [1 thread ] boost ublas mul    -> 3.10974 s.

  [float]  [1000x1000] [8 threads] haifisch mul       -> 0.70433 s.
  [float]  [1000x1000] [1 thread ] boost ublas mul    -> 25.2708 s.

  [float]  [1500x1500] [8 threads] haifisch mul       -> 3.10457 s.
  [float]  [1500x1500] [1 thread ] boost ublas mul    -> 83.8923 s.

  [double] [500x500]   [8 threads] haifisch mul       -> 0.08510 s.
  [double] [500x500]   [1 thread ] boost ublas mul    -> 3.41337 s.

  [double] [1000x1000] [8 threads] haifisch mul       -> 0.89308 s.
  [double] [1000x1000] [1 thread ] boost ublas mul    -> 26.4036 s.

  [double] [1500x1500] [8 threads] haifisch mul       -> 3.80144 s.
  [double] [1500x1500] [1 thread ] boost ublas mul    -> 92.1564 s.

```