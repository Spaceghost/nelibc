# Third party notices

## Unicode Character Database width properties

`src/locale_standalone.nelua` contains generated character-width ranges derived
from Unicode Character Database 16.0.0, including Python's Unicode properties
and `DerivedCoreProperties-16.0.0.txt`. The generator is
`tools/gen_wcwidth_data.py`. Unicode UCD 16 files are Copyright © 2024 Unicode,
Inc. Unicode License V3 is Copyright © 1991-2026 Unicode, Inc.; the applicable
permission notice is:

Permission is hereby granted, free of charge, to any person obtaining a copy of
data files and any associated documentation (the "Data Files") or software and
any associated documentation (the "Software") to deal in the Data Files or
Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, and/or sell copies of the Data Files
or Software, and to permit persons to whom the Data Files or Software are
furnished to do so, provided that either (a) this copyright and permission
notice appear with all copies of the Data Files or Software, or (b) this
copyright and permission notice appear in associated Documentation.

THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED
IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL
DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE DATA FILES OR
SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in these Data Files or Software without prior written authorization.

## fdlibm constants and math kernels

`src/math_core.nelua` includes constants and kernel coefficients originating
from fdlibm, as identified in that source file. The original notice grants
permission to use, copy, modify, and distribute them provided the notice is
preserved:

> Copyright (C) 1993 by Sun Microsystems, Inc.; developed at SunSoft.
> Permission to use, copy, modify, and distribute this software is freely
> granted, provided this notice is preserved.

The surrounding Nelibc project is distributed under the MIT License. This
notice continues to apply to the fdlibm-derived material.


## Lanczos gamma coefficients

`src/math_core.nelua` includes the Lanczos rational coefficients from musl's
`tgamma.c`, which is part of musl's MIT-licensed distribution. The applicable
notice is:

Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
