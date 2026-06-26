# Third-Party Notices

Alpha Stick's own code is MIT-licensed and its **original** hardware designs are
CERN-OHL-P v2 (see [LICENSE](LICENSE) and the [README](README.md#license)). This
repository also bundles one **third-party** design under a different licence,
listed here so the attribution is discoverable from the repository root. That
licence governs the component below; it is **not** overridden by the
repository's MIT / CERN-OHL-P licensing.

## Tetra II spherical flexure joint — the primary pivot mechanism

| | |
|---|---|
| **Author** | Jelle Rommers (`Jelle_Rommers`) |
| **Source** | Thingiverse [thing:4841850](https://www.thingiverse.com/thing:4841850) — *"Spherical flexure joint 1 and 2 (original files)"* |
| **Paper** | Rommers, van der Wijk, Herder, *"A new type of spherical flexure joint based on tetrahedron elements"*, Precision Engineering 71 (2021) 130–140, [doi:10.1016/j.precisioneng.2021.03.002](https://doi.org/10.1016/j.precisioneng.2021.03.002) |
| **Video** | https://www.youtube.com/watch?v=DAngcygU7tc |
| **Licence** | Creative Commons **Attribution (CC BY)**, as listed on the Thingiverse source. _Confirm the exact version/generation at the source before pinning it (see "To finish" below)._ |
| **Files in this repo** | [`models/tetra2-flexure/Tetra2.STEP`](models/tetra2-flexure/Tetra2.STEP), [`models/tetra2-flexure/Tetra2.STL`](models/tetra2-flexure/Tetra2.STL), and the `preview-*.png` renders |
| **Modifications** | **None.** These are the author's original published files, used unmodified. (An earlier parametric OpenSCAD *approximation* was removed in favour of the author's STEP — git commit `8079109` keeps it in history.) |

### If you modify it

CC BY lets you adapt and redistribute the part — including scaling it down or
recutting the stick mount, as the docs suggest — **provided you:**

1. keep the attribution to Jelle Rommers and the links above, and
2. **indicate that you changed it** (state what you altered).

A derivative does **not** become MIT or CERN-OHL-P just because it lives in this
repo; the CC BY attribution requirement travels with it. Per-part detail and
printing notes: [`models/tetra2-flexure/README.md`](models/tetra2-flexure/README.md).

### To finish (maintainer)

- Confirm the exact CC BY generation shown on the Thingiverse listing (e.g.
  CC BY 4.0) and pin it here and in the folder README; optionally drop the
  matching licence text into `models/tetra2-flexure/` as `LICENSE.CC-BY.txt`.
