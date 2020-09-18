proc-profile
============

Command Line Process Profiling Tool by David Catt, Bulat Ziganshin and Milian Wolff.

Example usage:

```
$ proc-profile my_benchmark.exe
...
Process ID       : 9848
Thread ID        : 10988
Process Exit Code: 0
Thread Exit Code : 0

User Time        :          13.390s
Kernel Time      :           5.109s
Process Time     :          18.499s
Clock Time       :          18.044s

Working Set      :         1087984 KB
Paged Pool       :            1317 KB
Nonpaged Pool    :              27 KB
Pagefile         :          866916 KB
Page Fault Count :         1031636
Page Size:       :               4 KB
Page Fault I/O   :         4126544 KB

IO Read          :          168029 KB (in           10853 reads )
IO Write         :            9090 KB (in             171 writes)
IO Other         :             872 KB (in           22523 others)
```
