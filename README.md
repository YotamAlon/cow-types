# python-cow

Copy-on-write alternatives to Python's built-in `dict`, `set`, and `list`.

These objects support all the same operations as their standard counterparts, but when passed to a function, any modifications made by the callee do not affect the caller's copy.
