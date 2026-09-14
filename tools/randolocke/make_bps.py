#!/usr/bin/env python3
"""Create and apply BPS patches, so a release can be cut without Flips installed.

    python3 tools/randolocke/make_bps.py create baserom.gba pokeemerald.gba out.bps
    python3 tools/randolocke/make_bps.py apply  baserom.gba out.bps rebuilt.gba
    python3 tools/randolocke/make_bps.py verify baserom.gba out.bps pokeemerald.gba

The encoder is a hash matcher: 32-byte windows of the source are indexed every 16 bytes,
target windows are looked up with a sorted search, and hits are extended in both
directions. Constant runs (the ROM's padding tail is ~8MB of one byte) become an
overlapping TargetCopy, which is the standard BPS way to spell run-length encoding.

`create` always verifies its own output by applying it back before writing.
"""
from __future__ import annotations

import sys
import zlib

import numpy as np

WINDOW = 32          # bytes that must match for a candidate to be considered
STRIDE = 16          # index every Nth source offset
MIN_MATCH = 16       # shorter matches cost more to encode than to inline
POLY = np.uint64(0x100000001B3)


# ---------------------------------------------------------------- varints ---

def encode_varint(number: int, out: bytearray) -> None:
    while True:
        x = number & 0x7F
        number >>= 7
        if number == 0:
            out.append(0x80 | x)
            return
        out.append(x)
        number -= 1


def encode_signed(number: int, out: bytearray) -> None:
    encode_varint((abs(number) << 1) | (1 if number < 0 else 0), out)


class Reader:
    def __init__(self, data: bytes, pos: int = 0):
        self.data, self.pos = data, pos

    def varint(self) -> int:
        data, shift = 0, 1
        while True:
            x = self.data[self.pos]
            self.pos += 1
            data += (x & 0x7F) * shift
            if x & 0x80:
                return data
            shift <<= 7
            data += shift

    def signed(self) -> int:
        v = self.varint()
        return -(v >> 1) if v & 1 else (v >> 1)


# ---------------------------------------------------------------- hashing ---

def rolling_hashes(buf: np.ndarray, window: int) -> np.ndarray:
    """FNV-style hash of every `window`-byte window, one entry per start offset."""
    n = len(buf) - window + 1
    if n <= 0:
        return np.zeros(0, dtype=np.uint64)
    h = np.zeros(n, dtype=np.uint64)
    for k in range(window):
        h = h * POLY + buf[k:k + n].astype(np.uint64)
    return h


# ---------------------------------------------------------------- encoder ---

def _emit(out: bytearray, length: int, action: int) -> None:
    encode_varint(((length - 1) << 2) | action, out)


def create(source: bytes, target: bytes, metadata: bytes = b"") -> bytes:
    src = np.frombuffer(source, dtype=np.uint8)
    tgt = np.frombuffer(target, dtype=np.uint8)
    ls, lt = len(src), len(tgt)

    print(f"  indexing {ls/1e6:.1f} MB source ...", flush=True)
    src_h = rolling_hashes(src, WINDOW)
    pos = np.arange(0, len(src_h), STRIDE, dtype=np.int64)
    keys = src_h[pos]
    order = np.argsort(keys, kind="stable")
    skeys, spos = keys[order], pos[order]
    del src_h, keys, order

    print(f"  scanning {lt/1e6:.1f} MB target ...", flush=True)
    tgt_h = rolling_hashes(tgt, WINDOW)
    ii = np.searchsorted(skeys, tgt_h)
    np.clip(ii, 0, len(skeys) - 1, out=ii)
    hit = skeys[ii] == tgt_h
    cand = np.where(hit, spos[ii], -1)
    del tgt_h, skeys, spos, ii, hit

    # Offsets that are worth stopping at: a source candidate, or the start of a run of
    # identical bytes long enough to be worth a TargetCopy.
    same_as_prev = np.zeros(lt, dtype=bool)
    same_as_prev[1:] = tgt[1:] == tgt[:-1]
    run_start = np.zeros(lt, dtype=bool)
    run_start[:-MIN_MATCH] = ~same_as_prev[:-MIN_MATCH]
    for k in range(1, MIN_MATCH):
        run_start[:-MIN_MATCH] &= same_as_prev[k:k - MIN_MATCH]
    interesting = np.zeros(lt, dtype=bool)
    interesting[:len(cand)] = cand >= 0
    interesting |= run_start
    stops = np.flatnonzero(interesting)
    del interesting, run_start

    out = bytearray(b"BPS1")
    encode_varint(ls, out)
    encode_varint(lt, out)
    encode_varint(len(metadata), out)
    out += metadata

    o = 0
    lit_start = 0
    src_rel = 0
    tgt_rel = 0
    si = 0
    stats = {"SourceRead": 0, "TargetRead": 0, "SourceCopy": 0, "TargetCopy": 0}

    def flush_literals(upto: int) -> None:
        nonlocal lit_start
        if upto > lit_start:
            n = upto - lit_start
            _emit(out, n, 1)
            out.extend(target[lit_start:upto])
            stats["TargetRead"] += n
        lit_start = upto

    while o < lt:
        si = np.searchsorted(stops, o, side="left")
        if si >= len(stops):
            break
        o = int(stops[si])

        best_len, best_src = 0, -1
        if o < len(cand) and cand[o] >= 0:
            s = int(cand[o])
            if bytes(src[s:s + WINDOW]) == target[o:o + WINDOW]:
                end = s + WINDOW
                oe = o + WINDOW
                limit = min(ls - end, lt - oe)
                if limit > 0:
                    a = src[end:end + limit]
                    b = tgt[oe:oe + limit]
                    diff = np.flatnonzero(a != b)
                    ext = int(diff[0]) if len(diff) else limit
                else:
                    ext = 0
                best_len, best_src = WINDOW + ext, s

        # A run of one repeated byte, encoded as a 1-byte literal plus an overlapping copy.
        rl = 0
        if o + MIN_MATCH <= lt:
            b0 = tgt[o]
            limit = min(lt - o, 1 << 22)
            neq = np.flatnonzero(tgt[o:o + limit] != b0)
            rl = int(neq[0]) if len(neq) else limit

        if best_len >= MIN_MATCH and best_len >= rl:
            flush_literals(o)
            if best_src == o:
                _emit(out, best_len, 0)
                stats["SourceRead"] += best_len
            else:
                _emit(out, best_len, 2)
                encode_signed(best_src - src_rel, out)
                src_rel = best_src + best_len
                stats["SourceCopy"] += best_len
            o += best_len
            lit_start = o
        elif rl >= MIN_MATCH:
            flush_literals(o + 1)          # the byte the run copies from
            n = rl - 1
            _emit(out, n, 3)
            encode_signed((o) - tgt_rel, out)
            tgt_rel = o + n
            stats["TargetCopy"] += n
            o += rl
            lit_start = o
        else:
            o += 1

    flush_literals(lt)

    out += zlib.crc32(source).to_bytes(4, "little")
    out += zlib.crc32(target).to_bytes(4, "little")
    out += zlib.crc32(bytes(out)).to_bytes(4, "little")
    total = sum(stats.values())
    for k, v in stats.items():
        print(f"    {k:11s} {v:12,d} bytes  {100*v/total:5.1f}%")
    return bytes(out)


# ---------------------------------------------------------------- decoder ---

def apply(source: bytes, patch: bytes) -> bytes:
    if patch[:4] != b"BPS1":
        raise SystemExit("not a BPS patch")
    if zlib.crc32(patch[:-4]) != int.from_bytes(patch[-4:], "little"):
        raise SystemExit("patch is corrupt (checksum mismatch)")
    r = Reader(patch, 4)
    src_size, tgt_size, meta_size = r.varint(), r.varint(), r.varint()
    r.pos += meta_size
    if len(source) != src_size:
        raise SystemExit(f"source is {len(source)} bytes, patch expects {src_size}")
    if zlib.crc32(source) != int.from_bytes(patch[-12:-8], "little"):
        raise SystemExit("source ROM does not match the one the patch was made from")

    out = bytearray(tgt_size)
    o = src_rel = tgt_rel = 0
    end = len(patch) - 12
    while r.pos < end:
        v = r.varint()
        length, action = (v >> 2) + 1, v & 3
        if action == 0:
            out[o:o + length] = source[o:o + length]
            o += length
        elif action == 1:
            out[o:o + length] = patch[r.pos:r.pos + length]
            r.pos += length
            o += length
        elif action == 2:
            src_rel += r.signed()
            out[o:o + length] = source[src_rel:src_rel + length]
            src_rel += length
            o += length
        else:
            tgt_rel += r.signed()
            for _ in range(length):
                out[o] = out[tgt_rel]
                o += 1
                tgt_rel += 1
    if zlib.crc32(bytes(out)) != int.from_bytes(patch[-8:-4], "little"):
        raise SystemExit("patched output does not match the expected checksum")
    return bytes(out)


def main() -> int:
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    mode = sys.argv[1]
    if mode == "create":
        base, built, out = sys.argv[2:5]
        source, target = open(base, "rb").read(), open(built, "rb").read()
        print(f"base   {base}  {len(source):,} bytes  crc32 {zlib.crc32(source)&0xffffffff:08x}")
        print(f"built  {built}  {len(target):,} bytes  crc32 {zlib.crc32(target)&0xffffffff:08x}")
        patch = create(source, target)
        print("  verifying by applying it back ...", flush=True)
        if apply(source, patch) != target:
            raise SystemExit("BUG: the patch does not reproduce the target")
        open(out, "wb").write(patch)
        print(f"wrote {out}  {len(patch):,} bytes "
              f"({100*len(patch)/len(target):.1f}% of the ROM)")
    elif mode == "apply":
        base, patch, out = sys.argv[2:5]
        data = apply(open(base, "rb").read(), open(patch, "rb").read())
        open(out, "wb").write(data)
        print(f"wrote {out}  crc32 {zlib.crc32(data)&0xffffffff:08x}")
    elif mode == "verify":
        base, patch, expect = sys.argv[2:5]
        data = apply(open(base, "rb").read(), open(patch, "rb").read())
        want = open(expect, "rb").read()
        print("MATCH" if data == want else "MISMATCH",
              f"crc32 {zlib.crc32(data)&0xffffffff:08x}")
        return 0 if data == want else 1
    else:
        raise SystemExit(__doc__)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
