# 🗂️ LocalRepoCactOS

<p align="center">
  <img src="https://img.shields.io/badge/license-GPLv3-blue.svg?style=for-the-badge" alt="License: GPLv3">
  <img src="https://img.shields.io/badge/output-localbinrepo.img-green.svg?style=for-the-badge" alt="localbinrepo.img">
  <img src="https://img.shields.io/badge/packer-Python%203-yellow.svg?style=for-the-badge" alt="Python 3">
  <img src="https://img.shields.io/badge/boot-Multiboot2%20module-purple.svg?style=for-the-badge" alt="Multiboot2 module">
  <img src="https://img.shields.io/badge/format-localbinrepo%20v1-orange.svg?style=for-the-badge" alt="localbinrepo v1">
  <img src="https://img.shields.io/badge/signature-ECDSA%20P--256-red.svg?style=for-the-badge" alt="ECDSA P-256">
</p>

<p align="center">
  Staging tree for <strong>early userspace</strong> ELFs, packed into one <strong><code>localbinrepo.img</code></strong> archive.<br>
  Out-of-tree drivers no longer ride it: they are packed into their own <strong>Multiboot2</strong> modules — <strong><code>drvrepo.tar.zst</code></strong> (all but network) and <strong><code>netdrvrepo.tar.zst</code></strong> (network).<br>
  GRUB loads it as a <strong>Multiboot2</strong> <strong><code>module2</code></strong>; the <strong>Cact</strong> kernel copies it into RAM <strong>before paging</strong> and overlays <strong><code>/lib</code></strong>, <strong><code>/bin</code></strong>, and <strong><code>/sbin</code></strong> on top of disk-backed VFS.<br>
  <strong>3.0.0:</strong> all `.cctk` modules are signed with an <strong>ECDSA P-256</strong> key pair. Each module carries a fixed trailer — magic `CMOD`, a 4-byte LE vermagic, then a 64-byte r‖s signature over `ELF || magic || vermagic`; the kernel verifies it with the embedded public key before loading (`mod_tag_verify()` in `Cact/kernel/elf/mod_tag.c`, Rust crate `cact_crypto`). The build host signs with `tools/modsign.py` via `tools/cact_sign.py`.
</p>

---

## 📊 Stats

| | |
|---|---|
| **Output image** | **`localbinrepo.img`** — flat archive, magic **`LBRP`** / version **1** ([`tools/localbinrepo.h`](tools/localbinrepo.h)) |
| **Packer** | **`tools/pack_localbinrepo.py`** — `python3 tools/pack_localbinrepo.py [-v] lib/ localbinrepo.img`; a live progress bar on a terminal, a line per 10% when the output is captured, and the full file table with **`-v`** ([`meson.build`](meson.build)) |
| **Driver slot** | **unused** — drivers ride their own boot modules (`drvrepo` / `netdrvrepo`); any **`lib/*.cctk`** staged anyway is signed and packed as **`/usr/lib/modules/<name>.cctk`** |
| **Userspace staging** | **`lib/bin/*`** → archive **`/usr/bin/*`** · **`lib/sbin/*`** → **`/usr/sbin/*`** (from **CactUserBins** **`stage`**) |
| **Dynamic libc** | **`lib/clibc.so`** — copied from **`<cactlib_dir>/build-meson/clibc.so`** (the **`cactlib_dir`** option, set by the integrator) |
| **Bootstrap ELFs** | **`init`** (copy of **cgoct**), **`cactsole`**, **`cgoct`**, **`cactsole-rescue`** (copy of **cactsole**) |
| **Merged layout** | userland is **usrmerge**: the kernel serves **`/usr/{bin,sbin,lib}`** and makes **`/bin`, `/sbin`, `/lib`** symlinks into them |

---

## 🔗 Ecosystem

| Piece | Role |
|-------|------|
| **[CactKernel-x86_64](https://github.com/QwaYer/CactKernel-x86_64)** | Parses the **`localbinrepo`** module, stages **`localbinrepo_stage[]`**, serves **GDD** / **pci_load_module**, **binfs** / **sbinfs** / **libfs** / **usrfs** overlays |
| **[Driver-for-Cact-x86_64](https://github.com/QwaYer/Driver-for-Cact-x86_64) / [Network-for-Cact-x86_64](https://github.com/QwaYer/Network-for-Cact-x86_64)** | Pack every **`*-for-Cact`** module into the **`drvrepo`** / **`netdrvrepo`** boot archives — no longer into **`lib/`** here |
| **[CactLib-x86_64](https://github.com/QwaYer/CactLibc-x86_64)** | Builds **`clibc.so`** consumed by staged ELFs |
| **[Cgoct-x86_64](https://github.com/QwaYer/Cgoct-x86_64)** | **`/usr/bin/init`** — userspace supervisor |
| **[Cactsole-x86_64](https://github.com/QwaYer/Cactsole-x86_64)** | **`/usr/bin/cactsole`** and **`/usr/bin/cactsole-rescue`** (same binary, two names) |
| **[CactUserBins-x86_64](https://github.com/QwaYer/CactUserBins-x86_64)** | **`ninja -C build-meson stage`** fills **`lib/bin/`** and **`lib/sbin/`** |
| **[CactOS-x86_64](https://github.com/QwaYer/CactOS-x86_64)** | **Workspace integrator** — drives **`ninja`** across libc, shells, userbins, drivers, this packer, kernel, **CactBridge** |

---

## 📦 What goes into `localbinrepo.img`

`lib/` is **staging only**: **`ninja stage`** fills it from the sibling trees and
nothing under it is ever committed (see [`.gitignore`](.gitignore)) — this
repository owns no sources and carries no binaries.

| Source under `lib/` | Path inside the archive | Purpose |
|---------------------|-------------------------|---------|
| **`*.cctk`** | **`/usr/lib/modules/<name>.cctk`** | Relocatable **PCI** driver blobs (**ET_REL**), loaded via **GDD** |
| **`*.so`** | **`/usr/lib/<name>.so`** | Shared libs (**libfs** overlay), e.g. **`clibc.so`** |
| **`ca-certificates.crt`** | **`/usr/share/ca-certificates.crt`** | Default **CA bundle** for the libc TLS client (libc looks in **`/etc/ssl/certs/ca-certificates.crt`** first and falls back to this copy) |
| **`consolefont.psf`** | **`/usr/share/consolefont.psf`** | PSF2 console font read by the kernel at boot. Generated — CactKernel's **`tools/gen_consolefont.py`** renders it deterministically, so the image needs no vendored copy |
| **`bin/*`** | **`/usr/bin/<name>`** | **init**, **cactsole**, **cgoct**, **cactsole-rescue**, plus all **CactUserBins** tools |
| **`sbin/*`** | **`/usr/sbin/<name>`** | Privileged / net helpers (**kill**, **su**, **modload**, **ping**, **cact-install**, …) |
| **`cactpkg/*`** | **`/usr/lib/cactpkg/repo/*`** | Offline **cactpkg** repository (base-system tools as `.cpkg`, versioned with the release) |
| **`cact-install/*`** | **`/usr/lib/cact-install/*`** | Installer media — **`boot/kernel.bin`** + the **GRUB** stage files the installer deploys, staged by **`cact-install`**'s **`stage-media`** (after its **`grub-bundle`**, with the kernel already built). The **image itself is not here**: the kernel serves the boot module as **`/dev/localbinrepo`** and `cact-install` reads it there, so it can never nest an image inside itself |

The packer sorts entries by **archive path**, then writes header + entry table + **NUL-separated** names (**8-byte** aligned) + payloads (**16-byte** aligned). See docstring in [`tools/pack_localbinrepo.py`](tools/pack_localbinrepo.py).

### The store and `firmrepo.tar.zst`

The store holds one thing: the **firmware mirror**. Every build wipes `store/`
and re-populates it with `cp -al` (hard links — the mirror and the store share
the same filesystem, so a 548 MiB tree is not duplicated), then
**`ninja -C build-meson firmrepo`** packs it into **`firmrepo.tar.zst`** (a
ustar image of `usr/…`, one zstd stream):

| Source under `store/` | Path inside the archive | Why it is not in the image |
|-----------------------|-------------------------|----------------------------|
| **`usr/lib/firmware/*`** | **`/usr/lib/firmware/*`** | The whole mirror, ~548 MiB; pulled per-blob on demand |

Blobs are copied **as-is** — the mirror ships `*.zst` (and `request_firmware()`
tries **`<name>.zst`** before **`<name>`**), so re-compressing buys nothing; the
archive's zstd level is low for the same reason. `tar` stores the mirror's
symlinks (aliases) as symlink members; `zstd -dc firmrepo.tar.zst | tar -xf -`
recreates them, so the aliases are reachable too.

The archive stays on the boot medium: the kernel serves the GRUB module in
place as **`/dev/firmrepo`**, and **`zstd`** + **`tar`** stream members out of it
(`zstd -dc /dev/firmrepo | tar -x - usr/lib/firmware/rt2870.bin`) without
unpacking the rest — the ~500 MiB archive is never copied into RAM.
`request_firmware()` then finds the extracted blob on the VFS.

---

## 🔨 Building

**Recommended — full workspace**

From the **parent** of all sibling trees, run **`ninja -C CactOS-x86_64/build-meson stage`** (or **`… iso`**) — **[CactOS-x86_64](https://github.com/QwaYer/CactOS-x86_64)** configures this project with **`-Dcactlib_dir`**, **`-Dcactsole_bin`**, **`-Dcgoct_bin`**, **`-Duserbins_mk`**, **`-Dcactsoleinc`** and runs its **`stage`** target, which repacks **`localbinrepo.img`**.

**Standalone — this repository only**

Sibling directories are auto-detected from their default relative paths:

```sh
meson setup build-meson
ninja -C build-meson stage   # gather siblings + pack localbinrepo.img
ninja -C build-meson ca      # re-stage just the CA bundle
ninja -C build-meson purge   # clean
```

Override any path with a Meson option (see table below).

| Option | Meaning |
|----------|---------|
| **`-Dcactlib_dir`** | Root of **CactLib-x86_64** (must already contain **`build-meson/clibc.so`**) |
| **`-Dcactsole_bin`** | Path to built **`cactsole`** |
| **`-Dcgoct_bin`** | Path to built **`cgoct`** |
| **`-Duserbins_mk`** | Directory of **CactUserBins-x86_64** (its **`stage`** target is invoked) |
| **`-Dcactsoleinc`** | **`include/`** from **Cactsole-x86_64** |

**Prerequisites**

| Requirement | Notes |
|-------------|-------|
| **`python3`** | Runs **`pack_localbinrepo.py`** |
| **`ninja stage`** | No drivers required — the image carries none (they ride `drvrepo` / `netdrvrepo`) |

Driver archives are packed by their aggregators, not here:

```sh
ninja -C ../Driver-for-Cact-x86_64/build-meson drvrepo      # → drvrepo.tar.zst
ninja -C ../Network-for-Cact-x86_64/build-meson netdrvrepo  # → netdrvrepo.tar.zst
```

**Pack the image**

```sh
ninja -C build-meson stage
ninja -C build-meson purge
```

---

## 📂 Repository layout

```
LocalRepoCactOS/
├── meson.build           # sibling paths arrive as -D options; stage/ca/purge targets
├── LICENSE
├── tools/
│   ├── localbinrepo.h          # on-disk layout (shared idea with kernel reader)
│   ├── pack_localbinrepo.py    # packs lib/ → localbinrepo.img (progress bar / -v)
│   └── cact_sign.py      # appends the ECDSA P-256 trailer to a .cctk
├── lib/                  # populated by the -D options / stage targets
│   ├── clibc.so
│   ├── ca-certificates.crt
│   ├── bin/
│   └── sbin/
├── src/                  # optional mirrors of driver sources
└── localbinrepo.img            # generated (gitignored in practice)
```

---

## 🚀 Boot flow (runtime)

1. **GRUB** reads **`grub.cfg`**:
   ```
   multiboot2 /boot/kernel.bin
   module2   /boot/localbinrepo.img localbinrepo
   module2   /boot/drvrepo.tar.zst  drvrepo
   module2   /boot/netdrvrepo.tar.zst netdrvrepo
   ```
2. Early **`init()`** (paging still off): **Multiboot2** parsing records the first module whose cmdline begins with **`localbinrepo`** (and, separately, **`drvrepo`** / **`netdrvrepo`**).
3. **`initfs_modblob_load(phys, size)`** copies the module into a static **`localbinrepo_stage[]`** **`.bss`** buffer **before** **`pmm_init_from_mmap()`** / **`init_heap()`** so the heap cannot overwrite bootloader pages.  The driver archives are **not** staged — they are read in place.
4. **`pci_enumerate()`** drives **GDD** prompts for recognised devices.
5. On confirm, **`pci_load_module("/usr/lib/modules/<name>.cctk", drv)`** resolves the path through **`modsrc_get()`** — the staged **`localbinrepo_stage`** buffer first, then the **`drvrepo`** archive (streamed on demand) — copies the **ET_REL** image, applies relocations, calls **`pci_driver_probe(dev)`**.
6. Userspace **`/usr/bin/init`** (**cgoct**) expects **`/usr/bin/cactsole`** (and optionally **`cactsole-rescue`**) on the overlay **`PATH`** (**`/usr/bin:/usr/sbin`**).

---

## ➕ Adding a new PCI driver

1. Create a sibling repo **`<Name>-for-Cact-x86_64`** whose **`meson.build`**:
   - compiles **`<name>_mod.c`** with **`-ffreestanding -fno-pie -mcmodel=kernel -mno-mmx -mno-sse -mno-sse2`** (ring-0 code must not use SSE),
   - emits **`<name>.cctk`** (relocatable object).
2. Add **`<Name>`** to the aggregator's **`drivers.list`** — **`Driver-for-Cact-x86_64`** (or **`Network-for-Cact-x86_64`** for network) — and to **`CactBridge-x86_64/drivers.list`**.
3. (Optional) mirror sources under **`src/<Name>-for-Cact/`**.
4. Teach **GDD** in **`CactKernel-x86_64/.../pci_gdd.c`** to recognise the PCI class tuple.
5. **`ninja -C Driver-for-Cact-x86_64/build-meson drvrepo`** packs it into the archive — or just run **`ninja -C CactOS-x86_64/build-meson drivers`**.

---

## ℹ️ Invariants

| Rule | Why |
|------|-----|
| **ABI** must match **libc** and the kernel | **`syscall.h`** (15 traps) and **`ioctl_abi.h`** are the contract — bump **CactLib**, then relink **cgoct**, **cactsole**, **CactUserBins** |
| **`/usr/bin/init` is cgoct** | The kernel’s first ELF task is **`usr/bin/init`**; keep this staging rule when swapping supervisors |
