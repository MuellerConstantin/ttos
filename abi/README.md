# TTOS - ABI

This subproject holds the binary interface between the kernel and userland.
Everything both sides have to agree on byte for byte belongs here, starting
with the structures a syscall fills in on behalf of its caller.

The kernel used to define those structures a second time next to its syscall
handlers, and libsys defined them in its public headers. Since the two include
paths are disjoint, no compiler ever compared the copies: adding a field on one
side only would have made the kernel write past the end of the caller's
structure, without a warning and without a fault at the point of the mistake.

The subproject contains headers only and produces no build artifact. The
kernel, libsys and the userland programs add `abi/include` to their include
path.
