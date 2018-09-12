# gadget_snapshots
Tools for [GADGET-2](http://wwwmpa.mpa-garching.mpg.de/gadget/) binaries (both SnapFormat=1 and SnapFormat=2)

*read_snapshot* has two modes of operation:

- `read_snapshot <snapshot>`
  
  Reads the `<snapshot>` file printing some information about it.

- `read_snapshot <src_snapshot> <dst_snapshot>` (SnapFormat=2 only)

  Reads the `<src_snapshot>` file and copies the desired tags to
  `<dst_snapshot>`. Currently, the list of tags which are allowed in the
  destination file is hard coded in the source code. Please check
  `read_snapshot.c` for instructions.
