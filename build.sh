#!/bin/sh
# Standalone POSIX verification build script generated from this repository's build tables.
# Requires only standard POSIX utilities and the user's own original PsyQ toolchain.
# Build mode: kit-libraries (game and non-kit libraries compiled from source; kit
# libraries linked from the original SDK archives; 6 stated module(s) totalling 12432
# bytes in config/lain/stated.tsv sliced from the retail executable).
set -eu

TARGET='lain'
CONFIG_DIR='config/lain'
BUILD_MODE='kit-libraries'
PRIMARY_KIT_VAR='PSYQ_4_3'
NEEDS_DOS=0
PSYLINK_BUCKET_WALK=0
HAS_ASSETS=1
HAS_STATED=1
COMMONS_ORDER='config/lain/commons.tsv'

usage() {
    cat <<'EOF'
Usage: sh build.sh [--build-dir DIR] [--report DIR] [PROGRAM...]

Environment variables (one per PsyQ kit release):
  PSYQ_4_3         Root directory of psyq-4.3 (holds PSX/INCLUDE and tools)
  PSYQ_ROOTS       Optional space-separated extra kit directories to search

Retail executables (for data arrays / stated sections):
  SLPS_EXE         Path to retail SLPS_016.03 (SLPS)
  ORIG_DIR         Optional directory searched for the retail executable(s)

Runners (on non-Windows hosts or for DOS tools):
  WIN32_RUNNER     Command prefix to run a Win32 executable (default: native on Windows).
                   Interface: $WIN32_RUNNER <exe> [args...] executed in the current directory.

Options:
  --build-dir DIR  Working/output directory (default: ./build-verify)
  --report DIR     Copy verified match report JSON(s) into DIR after SHA-256 passes
  PROGRAM...       Build only the named program(s); default builds all
EOF
}

export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL='*'
unset C_INCLUDE_PATH CPLUS_INCLUDE_PATH GCC_EXEC_PREFIX COMPILER_PATH SN_PATH PSYQ_PATH

IS_WINDOWS=0
case "$(uname -s 2>/dev/null || echo unknown)" in
    MINGW*|MSYS*|CYGWIN*) IS_WINDOWS=1 ;;
    *) [ "${OS:-}" = "Windows_NT" ] && IS_WINDOWS=1 ;;
esac

host_path() {
    if [ "$IS_WINDOWS" = "1" ] && command -v cygpath >/dev/null 2>&1; then
        cygpath -w "$1"
    else
        printf '%s\n' "$1"
    fi
}

file_sha256() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | awk '{print tolower($1)}'
    elif command -v shasum >/dev/null 2>&1; then
        shasum -a 256 "$1" | awk '{print tolower($1)}'
    else
        echo "error: sha256sum or shasum is required" >&2
        exit 1
    fi
}

run_win32() {
    if [ -n "${WIN32_RUNNER:-}" ]; then
        # shellcheck disable=SC2086
        $WIN32_RUNNER "$@"
    elif [ "$IS_WINDOWS" = "1" ]; then
        "$@"
    else
        echo "error: Win32 tool requires WIN32_RUNNER on this host (interface: \$WIN32_RUNNER <exe> [args...] in cwd)" >&2
        exit 1
    fi
}

run_dos() {
    if [ -n "${DOS_RUNNER:-}" ]; then
        # shellcheck disable=SC2086
        $DOS_RUNNER "$@"
    else
        echo "error: DOS tool requires DOS_RUNNER (interface: \$DOS_RUNNER <exe> [args...] in cwd)" >&2
        exit 1
    fi
}

run_tool() {
    _os="$1"; shift
    if [ "$_os" = "dos" ]; then
        run_dos "$@"
    else
        run_win32 "$@"
    fi
}

# Parse command-line arguments
BUILD_DIR="${BUILD_DIR:-}"
REPORT_DIR=""
WANT_BUILDS=""
while [ "$#" -gt 0 ]; do
    case "$1" in
        -h|--help) usage; exit 0 ;;
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --report) REPORT_DIR="$2"; shift 2 ;;
        -*) echo "error: unknown option $1" >&2; usage >&2; exit 2 ;;
        *) WANT_BUILDS="$WANT_BUILDS $1"; shift ;;
    esac
done

ROOT="$(pwd)"
[ -n "$BUILD_DIR" ] || BUILD_DIR="$ROOT/build-verify"
mkdir -p "$BUILD_DIR/tmp" "$BUILD_DIR/gen" "$BUILD_DIR/work" "$BUILD_DIR/out" "$BUILD_DIR/libobjs"
export TMPDIR="$(host_path "$BUILD_DIR/tmp")"

if [ "$NEEDS_DOS" = "1" ] && [ -z "${DOS_RUNNER:-}" ]; then
    echo "error: this target uses DOS period tools and requires DOS_RUNNER (interface: \$DOS_RUNNER <exe> [args...] in cwd)" >&2
    exit 1
fi

# Locate a tool by filename and SHA-256 across the given kit directories.
find_tool() {
    _fname="$1" _want="$2" _kvar="$3"; shift 3
    _listf="$BUILD_DIR/tmp/find_tool.list"
    _wrong=""
    for _dir in "$@"; do
        [ -n "$_dir" ] && [ -d "$_dir" ] || continue
        find "$_dir" -type f -iname "$_fname" 2>/dev/null > "$_listf" || true
        while IFS= read -r _cand; do
            [ -n "$_cand" ] || continue
            _got=$(file_sha256 "$_cand")
            if [ "$_got" = "$_want" ]; then
                printf '%s\n' "$_cand"
                return 0
            fi
            _wrong="$_wrong $_cand($_got)"
        done < "$_listf"
    done
    if [ -n "$_wrong" ]; then
        echo "error: $_fname expected sha256 $_want (set $_kvar), found mismatched:$_wrong" >&2
    else
        echo "error: $_fname (sha256 $_want) not found (set $_kvar or PSYQ_ROOTS)" >&2
    fi
    return 1
}

# Verify every required period tool by SHA-256
TOOL_ERR=0
TOOL_CPP_0=$(find_tool 'CPPPSX.EXE' '7becd3512121a037d6c413e35ec4d895c968ec2a52e84a24020e9e21e81a1919' PSYQ_4_3 "${PSYQ_4_3:-}" ${PSYQ_ROOTS:-}) || TOOL_ERR=1
TOOLOS_CPP_0=win32
TOOL_ASMPSX_1=$(find_tool 'ASMPSX.EXE' 'c4d686bd6ec9035b3b2295429e765f90b3d997a7f2666790008676ce4d0e85fa' PSYQ_4_3 "${PSYQ_4_3:-}" ${PSYQ_ROOTS:-}) || TOOL_ERR=1
TOOLOS_ASMPSX_1=win32
TOOL_DMPSX_2=$(find_tool 'DMPSX.EXE' 'd4b19e89d485fb5536303200f7dd53e328178d9b5e80fe079a7d4ce36955eb92' PSYQ_4_3 "${PSYQ_4_3:-}" ${PSYQ_ROOTS:-}) || TOOL_ERR=1
TOOLOS_DMPSX_2=win32
TOOL_LINK_3=$(find_tool 'PSYLINK.EXE' '0ee365fa3d31da71ba7086ccecd3d0df2fa39d4ad429e6a983fd9e4037272c6b' PSYQ_4_3 "${PSYQ_4_3:-}" ${PSYQ_ROOTS:-}) || TOOL_ERR=1
TOOLOS_LINK_3=win32
TOOL_CC1_4=$(find_tool 'CC1PSX.EXE' '17081b8e39a92078b16aa788f419306c0d77b28c30a02b9bb9c208cac172e5be' PSYQ_4_3 "${PSYQ_4_3:-}" ${PSYQ_ROOTS:-}) || TOOL_ERR=1
TOOLOS_CC1_4=win32
TOOL_AS_5=$(find_tool 'ASPSX.EXE' '60a72cc29aa556cd45db1224669583e0e00ecb7b19ecc2efa5da63a6e1598bbb' PSYQ_4_3 "${PSYQ_4_3:-}" ${PSYQ_ROOTS:-}) || TOOL_ERR=1
TOOLOS_AS_5=win32
[ "$TOOL_ERR" = "0" ] || { echo "error: one or more required period tools are missing or mismatched" >&2; exit 1; }

T_CPP="$TOOL_CPP_0"
T_ASMPSX="$TOOL_ASMPSX_1"
T_DMPSX="$TOOL_DMPSX_2"
T_LINK="$TOOL_LINK_3"
T_LINK_OS="$TOOLOS_LINK_3"

eval "PRIMARY_KIT=\${PSYQ_4_3:-}"
SDK_INC=""
for _root in "$PRIMARY_KIT" ${PSYQ_ROOTS:-}; do
    [ -n "$_root" ] && [ -d "$_root" ] || continue
    for _sub in "PSX/INCLUDE" "psx/include" "INCLUDE" "include"; do
        if [ -d "$_root/$_sub" ]; then SDK_INC="$_root/$_sub"; break 2; fi
    done
done
[ -n "$SDK_INC" ] || { echo "error: PSX/INCLUDE not found under $PRIMARY_KIT_VAR" >&2; exit 1; }

HDR_ERR=0
while read -r _hsha _hrel; do
    [ -n "$_hsha" ] || continue
    _hfile="$SDK_INC/$_hrel"
    if [ ! -f "$_hfile" ]; then
        _hfile="$SDK_INC/$(printf '%s' "$_hrel" | tr '[:upper:]' '[:lower:]')"
    fi
    if [ ! -f "$_hfile" ]; then
        echo "error: missing SDK header $_hrel in $SDK_INC" >&2
        HDR_ERR=1
        continue
    fi
    _got=$(file_sha256 "$_hfile")
    if [ "$_got" != "$_hsha" ]; then
        echo "error: SDK header $_hrel sha256 $_got != $_hsha" >&2
        HDR_ERR=1
    fi
done < "$CONFIG_DIR/headers.sha256"
[ "$HDR_ERR" = "0" ] || exit 1
echo "Verified tools and SDK headers."

SDK_LIB=""
for _root in "$PRIMARY_KIT" ${PSYQ_ROOTS:-}; do
    [ -n "$_root" ] && [ -d "$_root" ] || continue
    for _sub in "PSX/LIB" "psx/lib" "LIB" "lib"; do
        if [ -d "$_root/$_sub" ]; then SDK_LIB="$_root/$_sub"; break 2; fi
    done
done
[ -n "$SDK_LIB" ] || { echo "error: PSX/LIB not found under $PRIMARY_KIT_VAR" >&2; exit 1; }

find_kit_lib() {
    _file="$1" _want="$2"
    _cand="$SDK_LIB/$_file"
    if [ ! -f "$_cand" ]; then
        _cand="$SDK_LIB/$(printf '%s' "$_file" | tr '[:upper:]' '[:lower:]')"
    fi
    [ -f "$_cand" ] || { echo "error: missing kit library $_file in $SDK_LIB" >&2; return 1; }
    _got=$(file_sha256 "$_cand")
    [ "$_got" = "$_want" ] || { echo "error: kit library $_file sha256 $_got != $_want" >&2; return 1; }
    printf '%s\n' "$_cand"
}

read_u32_le() {
    _ru_f="$1" _ru_off="$2"
    dd if="$_ru_f" bs=1 skip="$_ru_off" count=4 2>/dev/null | LC_ALL=C od -An -tx1 -v | awk '
    BEGIN { hex = "0123456789ABCDEF" }
    {
        for (i=1; i<=NF; i++) {
            s = toupper($i)
            b[n++] = (index(hex, substr(s,1,1))-1)*16 + (index(hex, substr(s,2,1))-1)
        }
    }
    END { printf "%.0f\n", b[0] + b[1]*256 + b[2]*65536 + b[3]*16777216 }
    '
}

extract_slice() {
    # sh has no locals: names of its own, so a caller's `_out` stays its own.
    _xs_in="$1" _xs_off="$2" _xs_sz="$3" _xs_out="$4"
    if [ "$_xs_sz" -eq 0 ]; then
        : > "$_xs_out"
        return 0
    fi
    _xs_bs=4096
    _xs_full=$((_xs_off / _xs_bs))
    _xs_rem=$((_xs_off % _xs_bs))
    if [ "$_xs_rem" -eq 0 ] && [ $((_xs_sz % _xs_bs)) -eq 0 ]; then
        dd if="$_xs_in" of="$_xs_out" bs="$_xs_bs" skip="$_xs_full" count=$((_xs_sz / _xs_bs)) 2>/dev/null
    else
        dd if="$_xs_in" of="$_xs_out" bs=1 skip="$_xs_off" count="$_xs_sz" 2>/dev/null
    fi
}

hex_to_dec() {
    awk -v h="$1" 'BEGIN {
        hex = "0123456789ABCDEF"
        sub(/^0[xX]/, "", h); h = toupper(h); v = 0
        for (i=1; i<=length(h); i++) v = v*16 + index(hex, substr(h,i,1)) - 1
        printf "%.0f\n", v
    }'
}

# Parse SN LNK v2 object -> EXP, DEF, COM, REF, SEC, PLACEHOLDER lines
inspect_sn_obj() {
    LC_ALL=C od -An -tx1 -v "$1" | LC_ALL=C awk '
    BEGIN {
        hex = "0123456789ABCDEF"
        for (i=0; i<16; i++) h[substr(hex, i+1, 1)] = i
        n = 0
    }
    function hb(s) { s = toupper(s); return h[substr(s,1,1)]*16 + h[substr(s,2,1)] }
    function u16(p) { return b[p] + b[p+1]*256 }
    function u32(p) { return b[p] + b[p+1]*256 + b[p+2]*65536 + b[p+3]*16777216 }
    function pstr(p,   len, s, k) {
        len = b[p]; s = ""
        for (k=1; k<=len; k++) s = s sprintf("%c", b[p+k])
        pstr_next = p + 1 + len
        return s
    }
    function skip_expr(p,   op) {
        op = b[p]; p++
        if (op == 0) return p + 4
        if (op <= 24) return p + 2
        p = skip_expr(p)
        return skip_expr(p)
    }
    { for (i=1; i<=NF; i++) b[n++] = hb($i) }
    END {
        if (n < 4 || b[0] != 76 || b[1] != 78 || b[2] != 75 || b[3] != 2) exit 1
        p = 4; cur_sec = -1; has_ph = 0; nexp = 0; nsec = 0
        while (p < n) {
            tag = b[p]; p++
            if (tag == 0) break
            else if (tag == 2) {
                len = u16(p); p += 2
                if (cur_sec in sec_name && sec_name[cur_sec] == ".text") {
                    base = sec_sz[cur_sec]
                    for (off = 0; off + 4 <= len; off++) {
                        if ((base + off) % 4 == 0) {
                            w = u32(p + off)
                            if (int(w / 67108864) == 0 && (w % 64) == 63) has_ph = 1
                        }
                    }
                }
                if (cur_sec in sec_sz) sec_sz[cur_sec] += len
                p += len
            }
            else if (tag == 4) { p += 6 }
            else if (tag == 6) { cur_sec = u16(p); p += 2 }
            else if (tag == 8) { sz = u32(p); p += 4; if (cur_sec in sec_sz) sec_sz[cur_sec] += sz }
            else if (tag == 10) { p += 3; p = skip_expr(p) }
            else if (tag == 12) {
                sec = u16(p+2); nm = pstr(p+8); p = pstr_next
                exp_kind[++nexp] = "XDEF"; exp_name[nexp] = nm
                def_sec[nexp] = sec
            }
            else if (tag == 14) { nm = pstr(p+2); p = pstr_next; refs[++nref] = nm }
            else if (tag == 16) {
                sym = u16(p); nm = pstr(p+5); p = pstr_next
                sec_name[sym] = nm; sec_sz[sym] = 0; sec_ord[++nsec] = sym
            }
            else if (tag == 18 || tag == 40) { pstr(p+6); p = pstr_next }
            else if (tag == 20) { pstr(p+3); p = pstr_next }
            else if (tag == 22 || tag == 24 || tag == 26 || tag == 42) { p = skip_expr(p); p += 2 }
            else if (tag == 28) { pstr(p+2); p = pstr_next }
            else if (tag == 30) { p += 6 }
            else if (tag == 32) { p += 4 }
            else if (tag == 34) { }
            else if (tag == 36) { p += 1 }
            else if (tag == 38) { p += 2 }
            else if (tag == 44) { p += 3 }
            else if (tag == 46) { p += 1 }
            else if (tag == 48) {
                sec = u16(p+2); sz = u32(p+4); nm = pstr(p+8); p = pstr_next
                exp_kind[++nexp] = "XBSS"; exp_name[nexp] = nm
                com_sec[nexp] = sec; com_sz[nexp] = sz
            }
            else if (tag == 50 || tag == 60) { p += 2 }
            else if (tag == 52) { p += 3 }
            else if (tag == 54) { p += 4 }
            else if (tag == 56) { p += 6 }
            else if (tag == 58) { p += 8 }
            else if (tag == 62 || tag == 64 || tag == 66 || tag == 72) { p = skip_expr(p); p += 4 }
            else if (tag == 68) { p += 11 }
            else if (tag == 70) { p += 10 }
            else if (tag == 74) { pstr(p+24); p = pstr_next }
            else if (tag == 76 || tag == 78 || tag == 80) { p += 10 }
            else if (tag == 82) { pstr(p+14); p = pstr_next }
            else if (tag == 84) {
                nd = u16(p+14); p += 16 + nd*4
                pstr(p); p = pstr_next
                pstr(p); p = pstr_next
            }
            else exit 2
        }
        for (i=1; i<=nexp; i++) {
            print "EXP\t" exp_kind[i] "\t" exp_name[i]
            if (exp_kind[i] == "XDEF") print "DEF\t" sec_name[def_sec[i]] "\t" exp_name[i]
            else print "COM\t" sec_name[com_sec[i]] "\t" com_sz[i] "\t" exp_name[i]
        }
        for (i=1; i<=nref; i++) print "REF\t" refs[i]
        for (i=1; i<=nsec; i++) print "SEC\t" sec_name[sec_ord[i]] "\t" sec_sz[sec_ord[i]]
        if (has_ph) print "PLACEHOLDER"
    }'
}

# Pack SN LIB v1 archive (plain or marked commons). Skips 0-XDEF modules unless
# an explicit export override is supplied.
pack_lib_fast() {
    _out="$1" _mode="$2" _spec="$3"
    printf 'LIB\001' > "$_out"
    while IFS="$(printf '\t')" read -r _stem _obj _meta _expov; do
        [ -n "$_stem" ] || continue
        if [ "$_expov" = "-" ] || [ -z "$_expov" ]; then
            grep -q '^EXP	XDEF	' "$_meta" || continue
        fi
        _objsz=$(wc -c < "$_obj" | tr -d ' \r\n')
        if [ "$_expov" != "-" ] && [ -n "$_expov" ]; then
            printf '%s\n' "$_expov"
        else
            awk -F '\t' -v mode="$_mode" '$1=="EXP" { print ($2=="XBSS" && mode=="marked" ? "C" : "D") "\t" $3 }' "$_meta"
        fi | LC_ALL=C awk -F '\t' -v stem="$_stem" -v objsz="$_objsz" '
        BEGIN {
            ascii = " !\"#$%&'\''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
            s = toupper(stem); while (length(s)<8) s = s " "; s = substr(s, 1, 8)
            elen = 0
        }
        NF >= 2 {
            nm = $2; nl = length(nm)
            if ($1 == "C") { e[elen++] = nl + 1; e[elen++] = 0 }
            else { e[elen++] = nl }
            for (k=1; k<=nl; k++) e[elen++] = index(ascii, substr(nm, k, 1)) + 31
        }
        END {
            e[elen++] = 0
            hl = 20 + elen; ent = hl + objsz
            printf "%s%c%c%c%c", s, 0, 0, 0, 0
            printf "%c%c%c%c", hl%256, int(hl/256)%256, int(hl/65536)%256, int(hl/16777216)%256
            printf "%c%c%c%c", ent%256, int(ent/256)%256, int(ent/65536)%256, int(ent/16777216)%256
            for (i=0; i<elen; i++) printf "%c", e[i]
        }
        ' >> "$_out"
        cat "$_obj" >> "$_out"
    done < "$_spec"
}

# Convert PSYLINK CPE to PS-X EXE (1-pass pure POSIX awk)
cpe_to_exe_fast() {
    _cpe="$1" _exe="$2" _marker="$3"
    LC_ALL=C od -An -tx1 -v "$_cpe" | LC_ALL=C awk -v marker="$_marker" '
    BEGIN {
        hex = "0123456789ABCDEF"
        for (i=0; i<16; i++) h[substr(hex, i+1, 1)] = i
        n = 0; low = 4294967295; high = 0; pc = -1
        ascii = " !\"#$%&'\''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
    }
    function hb(s) { s = toupper(s); return h[substr(s,1,1)]*16 + h[substr(s,2,1)] }
    function u16(p) { return b[p] + b[p+1]*256 }
    function u32(p) { return b[p] + b[p+1]*256 + b[p+2]*65536 + b[p+3]*16777216 }
    function put32(arr, off, v) {
        arr[off] = v % 256; arr[off+1] = int(v / 256) % 256
        arr[off+2] = int(v / 65536) % 256; arr[off+3] = int(v / 16777216) % 256
    }
    { for (i=1; i<=NF; i++) b[n++] = hb($i) }
    END {
        if (n < 4 || b[0]!=67 || b[1]!=80 || b[2]!=69 || b[3]!=1) exit 1
        p = 4; nl = 0
        while (p < n) {
            t = b[p]; p++
            if (t == 0) break
            else if (t == 1) {
                a = u32(p); l = u32(p+4); p += 8
                if (l > 0) {
                    if (a < low) low = a
                    if (a + l > high) high = a + l
                    lf[++nl] = p; la[nl] = a; ll[nl] = l
                }
                p += l
            }
            else if (t == 2) p += 4
            else if (t == 3) { reg = u16(p); val = u32(p+2); p += 6; if (reg == 144) pc = val }
            else if (t == 4) p += 4
            else if (t == 5) p += 3
            else if (t == 6) p += 5
            else if (t == 7) p += 4
            else if (t == 8) p += 1
            else exit 1
        }
        sz = int((high - low + 2047) / 2048) * 2048
        for (i=0; i<2048; i++) hdr[i] = 0
        m = "PS-X EXE"
        for (i=1; i<=length(m); i++) hdr[i-1] = index(ascii, substr(m,i,1)) + 31
        put32(hdr, 16, pc); put32(hdr, 24, low); put32(hdr, 28, sz); put32(hdr, 48, 2149580784)
        for (i=1; i<=length(marker); i++) hdr[76 + i - 1] = index(ascii, substr(marker,i,1)) + 31
        for (i=0; i<2048; i++) printf "%c", hdr[i]
        for (i=1; i<=nl; i++) {
            off = la[i] - low; src = lf[i]; len = ll[i]
            for (k=0; k<len; k++) img[off + k] = b[src + k]
        }
        for (i=0; i<sz; i++) printf "%c", (i in img ? img[i] : 0)
    }
    ' > "$_exe"
}

emit_asset_inc() {
    _raw="$1" _out="$2" _layout="$3"
    LC_ALL=C od -An -tx1 -v "$_raw" | awk -v layout="$_layout" '
    BEGIN {
        hex = "0123456789ABCDEF"
        for (i=0; i<16; i++) h[substr(hex, i+1, 1)] = i
        n = 0; row_el = 0
        if (index(layout, ", rows of ") > 0) {
            split(layout, rp, ", rows of ")
            layout = rp[1]; row_el = rp[2] + 0
        }
        lang = "c"
        if (substr(layout, 1, 8) == "asm_psx ") { lang = "asm_psx"; layout = substr(layout, 9) }
        else if (substr(layout, 1, 4) == "asm ") { lang = "asm"; layout = substr(layout, 5) }
        split(layout, lp, " ")
        elem = lp[1]; endi = lp[2]
        w = (elem == "u8" || elem == "s8") ? 1 : ((elem == "u16" || elem == "s16") ? 2 : 4)
        cols = (w == 1) ? 16 : ((w == 2) ? 8 : 4)
    }
    function hb(s) { s = toupper(s); return h[substr(s,1,1)]*16 + h[substr(s,2,1)] }
    function fmt(v) {
        if (lang == "asm_psx") {
            if (w == 1) return sprintf("$%02X", v)
            if (w == 2) return sprintf("$%04X", v)
            return sprintf("$%08X", v)
        }
        if (w == 1) return sprintf("0x%02X", v)
        if (w == 2) return sprintf("0x%04X", v)
        return sprintf("0x%08X", v)
    }
    { for (i=1; i<=NF; i++) b[n++] = hb($i) }
    END {
        nel = int(n / w)
        for (i=0; i<nel; i++) {
            p = i * w
            if (w == 1) val = b[p]
            else if (w == 2) val = (endi == "big-endian") ? (b[p]*256 + b[p+1]) : (b[p] + b[p+1]*256)
            else val = (endi == "big-endian") ? (b[p]*16777216 + b[p+1]*65536 + b[p+2]*256 + b[p+3]) : (b[p] + b[p+1]*256 + b[p+2]*65536 + b[p+3]*16777216)
            els[i] = fmt(val)
        }
        if (lang == "c") {
            if (row_el > 0) {
                for (r=0; r<nel; r+=row_el) {
                    printf "    { "
                    for (j=0; j<row_el && r+j<nel; j++) printf "%s%s", (j>0?", ":""), els[r+j]
                    printf " }%s\n", (r+row_el<nel ? "," : "")
                }
            } else {
                for (i=0; i<nel; i+=cols) {
                    printf "    "
                    for (j=0; j<cols && i+j<nel; j++) {
                        sep = (j>0 ? ", " : "")
                        printf "%s%s", sep, els[i+j]
                    }
                    printf "%s\n", (i+cols<nel ? "," : "")
                }
            }
        } else {
            dir = (lang == "asm_psx") ? (w==1 ? "db" : (w==2 ? "dh" : "dw")) : (w==1 ? ".byte" : (w==2 ? ".half" : ".word"))
            for (i=0; i<nel; i+=cols) {
                printf "\t%s\t", dir
                for (j=0; j<cols && i+j<nel; j++) printf "%s%s", (j>0?", ":""), els[i+j]
                printf "\n"
            }
        }
    }' > "$_out"
}

materialize_assets() {
    _prog="$1" _exe="$2"
    [ -f "$CONFIG_DIR/assets.tsv" ] || return 0
    _taddr=$(read_u32_le "$_exe" 24)
    awk -F '\t' -v p="$_prog" '!/^#/ && $1!="include" && NF>=10 && $4==p { print $1 "\t" $5 "\t" $6 "\t" $8 "\t" $10 }' "$CONFIG_DIR/assets.tsv" > "$BUILD_DIR/tmp/assets.list"
    while IFS="$(printf '\t')" read -r _inc _addr _sz _asha _layout; do
        [ -n "$_inc" ] || continue
        _dec=$(hex_to_dec "$_addr")
        _off=$((2048 + _dec - _taddr))
        mkdir -p "$BUILD_DIR/gen/$(dirname "$_inc")"
        _raw="$BUILD_DIR/tmp/asset.bin"
        extract_slice "$_exe" "$_off" "$_sz" "$_raw"
        _got=$(file_sha256 "$_raw")
        [ "$_got" = "$_asha" ] || { echo "error: asset $_inc sha256 $_got != $_asha" >&2; exit 1; }
        emit_asset_inc "$_raw" "$BUILD_DIR/gen/$_inc" "$_layout"
    done < "$BUILD_DIR/tmp/assets.list"
}

# Build stated object(s) from stated.tsv and patch them into a kit LIB v1 archive.
build_stated_obj() {
    _prog="$1" _lib="$2" _mod="$3" _exe="$4" _as="$5" _asos="$6" _out="$7"
    _taddr=$(read_u32_le "$_exe" 24)
    _sfile="$BUILD_DIR/work/stated_${_lib}_${_mod}.s"
    : > "$_sfile"
    awk -F '\t' -v p="$_prog" -v l="$_lib" -v m="$_mod" \
        '!/^#/ && $1!="program" && NF>=9 && $1==p && $2==l && $3==m { print $4 "\t" $5 "\t" $6 "\t" $7 "\t" $8 "\t" $9 }' \
        "$CONFIG_DIR/stated.tsv" > "$BUILD_DIR/tmp/stated_rows.list"
    while IFS="$(printf '\t')" read -r _sec _addr _sz _ssha _exps _imps; do
        [ -n "$_sec" ] || continue
        printf '\t.section %s\r\n' "$_sec" >> "$_sfile"
        if [ "$_ssha" != "-" ] && [ -n "$_ssha" ]; then
            _dec=$(hex_to_dec "$_addr")
            _off=$((2048 + _dec - _taddr))
            _raw="$BUILD_DIR/tmp/stated.bin"
            extract_slice "$_exe" "$_off" "$_sz" "$_raw"
            _got=$(file_sha256 "$_raw")
            [ "$_got" = "$_ssha" ] || { echo "error: stated $_lib:$_mod $_sec sha256 $_got != $_ssha" >&2; exit 1; }
            LC_ALL=C od -An -tx1 -v "$_raw" | awk -v exps="$_exps" '
            BEGIN {
                n = 0
                if (exps != "-" && exps != "") {
                    ne = split(exps, ea, ",")
                    for (i=1; i<=ne; i++) {
                        split(ea[i], kv, "+")
                        off = kv[2] + 0
                        lbl[off] = (off in lbl ? lbl[off] "\t" : "") kv[1]
                    }
                }
            }
            { for (i=1; i<=NF; i++) b[n++] = toupper($i) }
            END {
                i = 0
                while (i < n) {
                    if (i in lbl) {
                        nl = split(lbl[i], la, "\t")
                        for (k=1; k<=nl; k++) printf "\t.globl %s\r\n%s%s\r\n", la[k], la[k], ":"
                    }
                    line = ""; cnt = 0
                    while (i < n && cnt < 16) {
                        if (cnt > 0 && (i in lbl)) break
                        line = (cnt > 0 ? line "," : "") "0x" b[i]
                        i++; cnt++
                    }
                    printf "\t.byte %s\r\n", line
                }
            }' >> "$_sfile"
        else
            awk -v sz="$_sz" -v exps="$_exps" 'BEGIN {
                if (exps != "-" && exps != "") {
                    ne = split(exps, ea, ",")
                    for (i=1; i<=ne; i++) {
                        split(ea[i], kv, "+")
                        off = kv[2] + 0
                        lbl[off] = (off in lbl ? lbl[off] "\t" : "") kv[1]
                    }
                }
                at = 0
                while (at < sz) {
                    if (at in lbl) {
                        nl = split(lbl[at], la, "\t")
                        for (k=1; k<=nl; k++) printf "\t.globl %s\r\n%s%s\r\n", la[k], la[k], ":"
                    }
                    nxt = sz
                    for (o in lbl) { oi = o + 0; if (oi > at && oi < nxt) nxt = oi }
                    printf "\t.space %.0f\r\n", nxt - at
                    at = nxt
                }
            }' >> "$_sfile"
        fi
        if [ "$_imps" != "-" ] && [ -n "$_imps" ]; then
            awk -v imps="$_imps" 'BEGIN {
                ni = split(imps, ia, ",")
                for (i=1; i<=ni; i++) if (ia[i] != "") printf "\t.extern %s,4\r\n", ia[i]
            }' >> "$_sfile"
        fi
    done < "$BUILD_DIR/tmp/stated_rows.list"
    (
        cd "$BUILD_DIR/work"
        run_tool "$_asos" "$_as" -q -G0 "stated_${_lib}_${_mod}.s" -o "stated_${_lib}_${_mod}.obj"
        inspect_sn_obj "stated_${_lib}_${_mod}.obj" > "${_out}.meta"
        cp "stated_${_lib}_${_mod}.obj" "$_out"
    )
}

patch_kit_lib() {
    _pk_kit="$1" _pk_out="$2" _pk_mode="$3" _pk_pmap="$4"
    _pk_idx="$BUILD_DIR/tmp/kit_idx.list"
    LC_ALL=C od -An -tx1 -v "$_pk_kit" | LC_ALL=C awk '
    BEGIN {
        hex = "0123456789ABCDEF"
        for (i=0; i<16; i++) h[substr(hex, i+1, 1)] = i
        n = 0
    }
    function hb(s) { s = toupper(s); return h[substr(s,1,1)]*16 + h[substr(s,2,1)] }
    function u32(p) { return b[p] + b[p+1]*256 + b[p+2]*65536 + b[p+3]*16777216 }
    { for (i=1; i<=NF; i++) b[n++] = hb($i) }
    END {
        if (n < 4 || b[0]!=76 || b[1]!=73 || b[2]!=66 || b[3]!=1) exit 1
        p = 4
        while (p < n) {
            nm = ""
            for (k=0; k<8; k++) nm = nm sprintf("%c", b[p+k])
            sub(/[ ]+$/, "", nm)
            ent = u32(p + 16)
            printf "%s\t%.0f\t%.0f\n", toupper(nm), p, ent
            p += ent
        }
    }' > "$_pk_idx"
    printf 'LIB\001' > "$_pk_out"
    while IFS="$(printf '\t')" read -r _pk_mname _pk_moff _pk_ment; do
        [ -n "$_pk_mname" ] || continue
        _pk_hit=$(awk -F '\t' -v m="$_pk_mname" '$1==m { print $2 "\t" $3; exit }' "$_pk_pmap")
        if [ -n "$_pk_hit" ]; then
            _pk_pobj=$(printf '%s\n' "$_pk_hit" | cut -f1)
            _pk_pmeta=$(printf '%s\n' "$_pk_hit" | cut -f2)
            _pk_objsz=$(wc -c < "$_pk_pobj" | tr -d ' \r\n')
            awk -F '\t' -v mode="$_pk_mode" '$1=="EXP" { print ($2=="XBSS" && mode=="marked" ? "C" : "D") "\t" $3 }' "$_pk_pmeta" | \
            LC_ALL=C awk -F '\t' -v stem="$_pk_mname" -v objsz="$_pk_objsz" '
            BEGIN {
                ascii = " !\"#$%&'\''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
                s = toupper(stem); while (length(s)<8) s = s " "; s = substr(s, 1, 8); elen = 0
            }
            NF >= 2 {
                nm = $2; nl = length(nm)
                if ($1 == "C") { e[elen++] = nl + 1; e[elen++] = 0 } else { e[elen++] = nl }
                for (k=1; k<=nl; k++) e[elen++] = index(ascii, substr(nm, k, 1)) + 31
            }
            END {
                e[elen++] = 0; hl = 20 + elen; ent = hl + objsz
                printf "%s%c%c%c%c", s, 0, 0, 0, 0
                printf "%c%c%c%c", hl%256, int(hl/256)%256, int(hl/65536)%256, int(hl/16777216)%256
                printf "%c%c%c%c", ent%256, int(ent/256)%256, int(ent/65536)%256, int(ent/16777216)%256
                for (i=0; i<elen; i++) printf "%c", e[i]
            }' >> "$_pk_out"
            cat "$_pk_pobj" >> "$_pk_out"
        else
            extract_slice "$_pk_kit" "$_pk_moff" "$_pk_ment" "$BUILD_DIR/tmp/kit_mod.bin"
            cat "$BUILD_DIR/tmp/kit_mod.bin" >> "$_pk_out"
        fi
    done < "$_pk_idx"
}

H_GEN="$(host_path "$BUILD_DIR/gen")"
H_SDK="$(host_path "$SDK_INC")"

compile_c() {
    _src="$1" _out="$2" _cc1="$3" _cc1os="$4" _cflags="$5" _as="$6" _asos="$7" _div="$8" _pflag="$9"
    shift 9
    _def="$1" _hinc="$2"
    _w="$BUILD_DIR/work"
    _sdir="$(host_path "$(dirname "$_src")")"
    _gflag=$(printf '%s\n' "$_cflags" | awk '{for(i=1;i<=NF;i++) if($i~/^-G[0-9]+$/) print $i}')
    _opt_def=""
    case " $_cflags " in *" -O0 "*) ;; *) _opt_def="-D__OPTIMIZE__" ;; esac
    _dflag=""
    [ -n "$_def" ] && [ "$_def" != "-" ] && _dflag="-D$_def"
    _divflag=""
    [ "$_div" = "no" ] && _divflag="-0"
    _as_p=""
    [ "$_pflag" = "yes" ] && _as_p="-P"
    (
        cd "$_w"
        # shellcheck disable=SC2086
        run_win32 "$T_CPP" -undef -lang-c \
            -D__GNUC__=2 -Dmips -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
            -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D__CHAR_UNSIGNED__ -D_LANGUAGE_C -DLANGUAGE_C \
            $_opt_def $_dflag \
            "-I${_sdir}/" "-I${_hinc}/" "-I${H_GEN}/" "-I${H_SDK}/" \
            "$(host_path "$_src")" "in.i"
        # shellcheck disable=SC2086
        run_tool "$_cc1os" "$_cc1" -quiet $_cflags "in.i" -o "in.s"
        # shellcheck disable=SC2086
        run_tool "$_asos" "$_as" -q $_as_p $_gflag $_divflag "in.s" -o "out.obj"
        inspect_sn_obj "out.obj" > "out.meta"
        if [ -n "$T_DMPSX" ] && grep -q '^PLACEHOLDER' "out.meta"; then
            run_win32 "$T_DMPSX" "out.obj" -b >/dev/null
            inspect_sn_obj "out.obj" > "out.meta"
        fi
        cp "out.obj" "$_out"
        cp "out.meta" "${_out}.meta"
    )
}

assemble_s() {
    _src="$1" _out="$2" _as="$3" _asos="$4" _cflags="$5" _div="$6" _pflag="$7" _def="$8" _hinc="$9"
    _w="$BUILD_DIR/work"
    _sdir="$(host_path "$(dirname "$_src")")"
    _gflag=$(printf '%s\n' "$_cflags" | awk '{for(i=1;i<=NF;i++) if($i~/^-G[0-9]+$/) print $i}')
    _opt_def=""
    case " $_cflags " in *" -O0 "*) ;; *) _opt_def="-D__OPTIMIZE__" ;; esac
    _divflag=""
    [ "$_div" = "no" ] && _divflag="-0"
    _as_p=""
    [ "$_pflag" = "yes" ] && _as_p="-P"
    (
        cd "$_w"
        cp "$_src" "raw.s"
        _in="raw.s"
        if grep -Eq '^[[:space:]]*#[[:space:]]*(include|define|undef|if|ifdef|ifndef|elif|else|endif)\b' "raw.s"; then
            _dflag=""
            [ -n "$_def" ] && [ "$_def" != "-" ] && _dflag="-D$_def"
            # shellcheck disable=SC2086
            run_win32 "$T_CPP" -undef -lang-asm -traditional -P \
                -D__GNUC__=2 -Dmips -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
                -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D__CHAR_UNSIGNED__ -D_LANGUAGE_ASSEMBLY -DLANGUAGE_ASSEMBLY \
                $_opt_def $_dflag \
                "-I${_sdir}/" "-I${_hinc}/" "-I${H_GEN}/" "-I${H_SDK}/" \
                "raw.s" "pp.s"
            _in="pp.s"
        fi
        if [ -z "$T_ASMPSX" ] || grep -Eiq '^[[:space:]]*\.globl\b' "$_in"; then
            # shellcheck disable=SC2086
            run_tool "$_asos" "$_as" -q $_as_p $_gflag $_divflag "$_in" -o "out.obj"
            inspect_sn_obj "out.obj" > "out.meta"
            if [ -n "$T_DMPSX" ] && grep -q '^PLACEHOLDER' "out.meta"; then
                run_win32 "$T_DMPSX" "out.obj" -b >/dev/null
                inspect_sn_obj "out.obj" > "out.meta"
            fi
        else
            _eflag=""
            if [ -n "$_def" ] && [ "$_def" != "-" ]; then
                case "$_def" in
                    *=*) _eflag="/e $_def" ;;
                    *) _eflag="/e ${_def}=1" ;;
                esac
            fi
            printf '/l /q %s /j %s /j %s /j %s /j %s %s,out.obj\r\n' "$_eflag" "$_sdir" "$_hinc" "$H_GEN" "$H_SDK" "$_in" > "asmpsx.rsp"
            run_win32 "$T_ASMPSX" "@asmpsx.rsp"
            inspect_sn_obj "out.obj" > "out.meta"
            if [ -n "$T_DMPSX" ] && grep -Eiq '^[[:space:]]*#?[[:space:]]*include\b' "raw.s" && grep -q '^PLACEHOLDER' "out.meta"; then
                run_win32 "$T_DMPSX" "out.obj" -b >/dev/null
                inspect_sn_obj "out.obj" > "out.meta"
            fi
        fi
        cp "out.obj" "$_out"
        cp "out.meta" "${_out}.meta"
    )
}

select_program_recipe() {
    case "$1" in
        *)
            R_CC1="$TOOL_CC1_4"; R_CC1OS="$TOOLOS_CC1_4"; R_CFLAGS='-O2 -G8'; R_AS="$TOOL_AS_5"; R_ASOS="$TOOLOS_AS_5"; R_DIV=yes; R_PFLAG=no
            ;;
    esac
}

select_library_recipe() {
    _lib="$1" _mod="$2"
    case "$_lib" in
        'NITRBS')
            R_CC1="$TOOL_CC1_4"; R_CC1OS="$TOOLOS_CC1_4"; R_CFLAGS='-O0 -G0'; R_AS="$TOOL_AS_5"; R_ASOS="$TOOLOS_AS_5"; R_DIV=yes; R_PFLAG=no
            ;;
        *)
            echo "error: no compiled library recipe for $_lib" >&2
            exit 1
            ;;
    esac
}

emit_order_sbss_bss() {
    _prog="$1" _meta="$2" _incmeta="$3" _out="$4" _bw="$5"
    awk -F '\t' -v prog="$_prog" -v bw="$_bw" '
    BEGIN { mode = 0; col = 0; s_n = 0; b_n = 0; pre_sbss = 0 }
    FILENAME == ARGV[1] {
        if ($1 == "DEF") def[$3] = 1
        else if ($1 == "REF") ref[$2] = 1
        else if ($1 == "COM") {
            com[$4] = 1
            sz = $3 + 0
            if (!($4 in com_sz) || sz > com_sz[$4]) com_sz[$4] = sz
            com_sec[$4, $2] = 1
        }
        next
    }
    FILENAME == ARGV[2] {
        if ($2 == "SEC" && $3 == ".sbss") {
            inc_sbss[++nsb] = $4 + 0
        }
        next
    }
    !/^#/ && NF >= 3 {
        if ($1 == "section" && $2 == "name") {
            for (i=3; i<=NF; i++) if ($i == prog) col = i
            next
        }
        if (col == 0) next
        cell = $col
        if (cell == "-") next
        sec = $1; nm = $2
        if (bw == 0 && sec != ".sbss") next
        split(cell, p, "+")
        addr = 0; h = toupper(p[1]); hex = "0123456789ABCDEF"
        for (i=1; i<=length(h); i++) addr = addr*16 + index(hex, substr(h,i,1)) - 1
        sz = p[2] + 0
        if (sec == ".sbss") { s_n++; s_nm[s_n]=nm; s_ad[s_n]=addr; s_sz[s_n]=sz; listed_sbss[nm]=1 }
        else if (sec == ".bss") { b_n++; b_nm[b_n]=nm; b_ad[b_n]=addr; b_sz[b_n]=sz }
    }
    function uninit_align(sz) { return sz<=1?1:(sz<=2?2:(sz<=4?4:8)) }
    function align_up(v, a) { return int((v + a - 1) / a) * a }
    function emit_sec(sec, cap, cnt, nms, ads, szs,   k, at, addr, end, stop, rest, al, chunk, fits, lb, tmp, present, oa, ra, sym, a, po, pa, wanted, actual) {
        if (cnt == 0) return
        printf "\t.section %s\r\n", sec
        if (!emitted_hdr) { printf "\t.globl __commons_order\r\n__commons_order%s\r\n", ":"; emitted_hdr = 1 }
        if (bw == 0 && sec == ".sbss") {
            oa = 4; ra = 4
            for (sym in com) {
                if (sym in def) continue
                if (!((sym SUBSEP ".sbss") in com_sec)) continue
                a = uninit_align(com_sz[sym])
                if (a > oa) oa = a
                if (!(sym in listed_sbss) && a > ra) ra = a
            }
            if (oa > ra) {
                po = 0; pa = 0
                for (k=1; k<=nsb; k++) {
                    po = align_up(po, oa) + inc_sbss[k]
                    pa = align_up(pa, ra) + inc_sbss[k]
                }
                wanted = align_up(po, oa); actual = align_up(pa, ra)
                if (wanted > actual) printf "\t.space %.0f\r\n", wanted - actual
            }
        }
        at = int(ads[1] / 4) * 4
        for (k=1; k<=cnt; k++) {
            addr = ads[k]; end = addr + szs[k]
            present = (((nms[k] in com) || (nms[k] in ref)) && !(nms[k] in def))
            while (at < end) {
                stop = (at < addr) ? addr : end
                rest = stop - at
                if (rest > cap) rest = cap
                al = uninit_align(rest)
                if (at % al == 0) chunk = rest
                else {
                    fits = 1; while (fits * 2 <= rest) fits *= 2
                    lb = 1; tmp = at
                    while (lb < 8 && (tmp % 2) == 0) { lb *= 2; tmp = int(tmp / 2) }
                    chunk = (lb < fits) ? lb : fits
                }
                if (present && at == addr) printf "\t.globl %s\r\n%s%s\r\n", nms[k], nms[k], ":"
                printf "\t.space %.0f\r\n", chunk
                at += chunk
            }
        }
    }
    END {
        emitted_hdr = 0
        emit_sec(".sbss", 8, s_n, s_nm, s_ad, s_sz)
        if (bw == 1) emit_sec(".bss", 4294967295, b_n, b_nm, b_ad, b_sz)
    }
    ' "$_meta" "$_incmeta" "$COMMONS_ORDER" > "$_out"
}

emit_per_object_bss() {
    _prog="$1" _incmeta="$2" _outdir="$3"
    awk -F '\t' -v prog="$_prog" -v outdir="$_outdir" '
    BEGIN { col = 0 }
    FILENAME == ARGV[1] {
        idx = $1 + 0
        if ($2 == "DEF") def[$4] = 1
        else if ($2 == "COM" && $3 == ".bss") {
            sz = $4 + 0; nm = $5
            if (!(nm in own) || sz > own_sz[nm]) { own[nm] = idx; own_sz[nm] = sz }
        }
        next
    }
    !/^#/ && NF >= 3 {
        if ($1 == "section" && $2 == "name") {
            for (i=3; i<=NF; i++) if ($i == prog) col = i
            next
        }
        if (col == 0 || $1 != ".bss" || $col == "-") next
        nm = $2
        if ((nm in def) || !(nm in own)) next
        split($col, p, "+")
        addr = 0; h = toupper(p[1]); hex = "0123456789ABCDEF"
        for (i=1; i<=length(h); i++) addr = addr*16 + index(hex, substr(h,i,1)) - 1
        o = own[nm]
        cnt[o]++
        k = cnt[o]
        o_ad[o, k] = addr; o_nm[o, k] = nm; o_sz[o, k] = own_sz[nm]
    }
    function uninit_align(sz) { return sz<=1?1:(sz<=2?2:(sz<=4?4:8)) }
    END {
        for (o in cnt) {
            n = cnt[o]
            if (n < 2) continue
            for (i=1; i<=n; i++) {
                for (j=i+1; j<=n; j++) {
                    if (o_ad[o, j] < o_ad[o, i]) {
                        t = o_ad[o, i]; o_ad[o, i] = o_ad[o, j]; o_ad[o, j] = t
                        t = o_nm[o, i]; o_nm[o, i] = o_nm[o, j]; o_nm[o, j] = t
                        t = o_sz[o, i]; o_sz[o, i] = o_sz[o, j]; o_sz[o, j] = t
                    }
                }
            }
            f = outdir "/ob" o ".s"
            printf "\t.section .bss\r\n" > f
            cur = 0
            for (i=1; i<=n; i++) {
                sz = o_sz[o, i]; al = uninit_align(sz)
                nxt = int((cur + al - 1) / al) * al
                if (nxt > cur) { printf "\t.space %.0f\r\n", nxt - cur > f; cur = nxt }
                printf "\t.globl %s\r\n%s%s\r\n\t.space %.0f\r\n", o_nm[o, i], o_nm[o, i], ":", sz > f
                cur += sz
            }
            close(f)
            print o
        }
    }' "$_incmeta" "$COMMONS_ORDER"
}

H_LIB_INC="$(host_path "$ROOT/include")"
# Build source libraries once (shared across builds)
LIB_BUILT_COUNT=0
LIB_MOD_COUNT=0
awk -F '\t' -v mode="$BUILD_MODE" \
    '!/^#/ && $1!="library" && $2!="-" && (mode=="full-source" || $3=="-") { print $1 "\t" $2 "\t" $5 }' \
    "$CONFIG_DIR/libraries.tsv" > "$BUILD_DIR/tmp/src_libs.list"

while IFS="$(printf '\t')" read -r _lib _lst _clisting; do
    [ -n "$_lib" ] || continue
    _ldir="$(dirname "$_lst")"
    _outlib="$BUILD_DIR/libobjs/${_lib}.LIB"
    _spec="$BUILD_DIR/libobjs/${_lib}.spec"
    : > "$_spec"
    awk '!/^[[:space:]]*;/ && NF>0 { gsub(/\r/, ""); print $1 }' "$_lst" > "$BUILD_DIR/tmp/lib_members.list"
    while IFS= read -r _mobj; do
        [ -n "$_mobj" ] || continue
        _stem="${_mobj%.obj}"
        _mname=$(basename "$_stem")
        _flat=$(printf '%s' "$_stem" | tr '/\\' '__')
        _dst="$BUILD_DIR/libobjs/${_lib}_${_flat}.obj"
        select_library_recipe "$_lib" "$(printf '%s' "$_mname" | tr '[:lower:]' '[:upper:]')"
        if [ -f "$ROOT/$_ldir/${_stem}.c" ]; then
            compile_c "$ROOT/$_ldir/${_stem}.c" "$_dst" "$R_CC1" "$R_CC1OS" "$R_CFLAGS" "$R_AS" "$R_ASOS" "$R_DIV" "$R_PFLAG" "-" "$H_LIB_INC"
        elif [ -f "$ROOT/$_ldir/${_stem}.s" ]; then
            assemble_s "$ROOT/$_ldir/${_stem}.s" "$_dst" "$R_AS" "$R_ASOS" "$R_CFLAGS" "$R_DIV" "$R_PFLAG" "-" "$H_LIB_INC"
        else
            echo "error: missing library source for $_lib:$_stem in $_ldir" >&2
            exit 1
        fi
        printf '%s\t%s\t%s\t-\n' "$_mname" "$_dst" "${_dst}.meta" >> "$_spec"
        LIB_MOD_COUNT=$((LIB_MOD_COUNT + 1))
    done < "$BUILD_DIR/tmp/lib_members.list"
    pack_lib_fast "$_outlib" "$_clisting" "$_spec"
    LIB_BUILT_COUNT=$((LIB_BUILT_COUNT + 1))
    echo "  built library ${_lib}.LIB"
done < "$BUILD_DIR/tmp/src_libs.list"

# Locate a retail executable from its env var or ORIG_DIR
find_orig_exe() {
    _evar="$1" _ename="$2" _eself="$3"
    if [ -n "$_eself" ] && [ -f "$_eself" ]; then
        printf '%s\n' "$_eself"
        return 0
    fi
    if [ -n "${ORIG_DIR:-}" ] && [ -d "$ORIG_DIR" ]; then
        _listf="$BUILD_DIR/tmp/find_exe.list"
        find "$ORIG_DIR" -type f -iname "$_ename" 2>/dev/null > "$_listf" || true
        while IFS= read -r _cand; do
            [ -n "$_cand" ] || continue
            printf '%s\n' "$_cand"
            return 0
        done < "$_listf"
    fi
    echo "error: retail $_ename not found (set $_evar=/path/to/$_ename or ORIG_DIR)" >&2
    return 1
}

build_program() {
    _prog="$1" _exename="$2" _evar="$3" _eself="$4" _cmdfile="$5" _objsin="$6" _hdrsin="$7" _def="$8" _wantsha="$9"
    shift 9
    _marker="$1"

    if [ -n "$WANT_BUILDS" ]; then
        case " $WANT_BUILDS " in
            *" $_prog "*|*" $_exename "*) ;;
            *) return 0 ;;
        esac
    fi

    echo "Building $_prog ($_exename)..."
    _exe_in=""
    if [ "$HAS_ASSETS" = "1" ] || [ "$HAS_STATED" = "1" ]; then
        _exe_in=$(find_orig_exe "$_evar" "$_exename" "$_eself")
    fi
    if [ "$HAS_ASSETS" = "1" ]; then
        materialize_assets "$_prog" "$_exe_in"
    fi

    _hinc="$(host_path "$ROOT/$_hdrsin")"
    select_program_recipe "$_prog"
    _ldir="$BUILD_DIR/link_${_prog}"
    rm -rf "$_ldir" && mkdir -p "$_ldir"

    # Compile every included unit in command-file order
    _oi=0
    : > "$_ldir/all.meta"
    : > "$_ldir/inc.meta"
    awk '!/^[[:space:]]*;/ && /^[[:space:]]*include[[:space:]]+"/' "$_cmdfile" | \
        sed -e 's/.*include[[:space:]]*"\([^"]*\)".*/\1/' -e 's/\r$//' > "$_ldir/includes.list"
    while IFS= read -r _relobj; do
        [ -n "$_relobj" ] || continue
        _stem="${_relobj%.obj}"
        _dst="$_ldir/o${_oi}.obj"
        if [ -f "$ROOT/$_objsin/${_stem}.c" ]; then
            compile_c "$ROOT/$_objsin/${_stem}.c" "$_dst" "$R_CC1" "$R_CC1OS" "$R_CFLAGS" "$R_AS" "$R_ASOS" "$R_DIV" "$R_PFLAG" "$_def" "$_hinc"
        elif [ -f "$ROOT/$_objsin/${_stem}.s" ]; then
            assemble_s "$ROOT/$_objsin/${_stem}.s" "$_dst" "$R_AS" "$R_ASOS" "$R_CFLAGS" "$R_DIV" "$R_PFLAG" "$_def" "$_hinc"
        else
            echo "error: missing source for $_objsin/$_stem" >&2
            exit 1
        fi
        cat "${_dst}.meta" >> "$_ldir/all.meta"
        awk -v idx="$_oi" '{ print idx "\t" $0 }' "${_dst}.meta" >> "$_ldir/inc.meta"
        _oi=$((_oi + 1))
    done < "$_ldir/includes.list"

    # Append packed source-library metas (skipping 0-XDEF modules) so commons order sees them
    for _spec in "$BUILD_DIR"/libobjs/*.spec; do
        [ -f "$_spec" ] || continue
        while IFS="$(printf '\t')" read -r _ms _mo _mm _me; do
            [ -n "$_mm" ] && [ -f "$_mm" ] || continue
            grep -q '^EXP	XDEF	' "$_mm" || continue
            cat "$_mm" >> "$_ldir/all.meta"
        done < "$_spec"
    done

    # Per-object .bss ordering for PSYLINK 2.73+ (bucket_walk = 0)
    : > "$_ldir/ob_idx.list"
    if [ -n "$COMMONS_ORDER" ] && [ "$PSYLINK_BUCKET_WALK" = "0" ]; then
        emit_per_object_bss "$_prog" "$_ldir/inc.meta" "$_ldir" > "$_ldir/ob_idx.list"
        while IFS= read -r _ob; do
            [ -n "$_ob" ] || continue
            (
                cd "$_ldir"
                run_tool "$R_ASOS" "$R_AS" -q -G8 "ob${_ob}.s" -o "ob${_ob}.obj"
            )
        done < "$_ldir/ob_idx.list"
    fi

    # Build link.lnk preserving non-input directives from the command file
    awk '!/^[[:space:]]*;/ && !/^[[:space:]]*$/ && !/^[[:space:]]*(include|inclib|regs)[[:space:]]/ { gsub(/\r/, ""); printf "%s\r\n", $0 }' "$_cmdfile" > "$_ldir/link.lnk"
    _k=0
    while [ "$_k" -lt "$_oi" ]; do
        printf '\tinclude\t"o%d.obj"\r\n' "$_k" >> "$_ldir/link.lnk"
        if grep -qx "$_k" "$_ldir/ob_idx.list" 2>/dev/null; then
            printf '\tinclude\t"ob%d.obj"\r\n' "$_k" >> "$_ldir/link.lnk"
        fi
        _k=$((_k + 1))
    done

    # Global commons order library (order.obj + order.lib)
    _has_order=0
    if [ -n "$COMMONS_ORDER" ]; then
        emit_order_sbss_bss "$_prog" "$_ldir/all.meta" "$_ldir/inc.meta" "$_ldir/order.s" "$PSYLINK_BUCKET_WALK"
        if [ -s "$_ldir/order.s" ]; then
            _has_order=1
            (
                cd "$_ldir"
                run_tool "$R_ASOS" "$R_AS" -q -G8 "order.s" -o "order_def.obj"
                printf '\t.extern __commons_order,4\r\n' > "ref.s"
                run_tool "$R_ASOS" "$R_AS" -q -G8 "ref.s" -o "order.obj"
                printf 'COMMONS\t%s\t-\tD\t__commons_order\n' "$_ldir/order_def.obj" > "order.spec"
                pack_lib_fast "order.lib" "plain" "order.spec"
            )
            printf '\tinclude\t"order.obj"\r\n' >> "$_ldir/link.lnk"
        fi
    fi

    # Resolve libraries in command-file order
    _li=0
    _kit_libs=0
    _stated_mods=0
    _stated_bytes=0
    awk '!/^[[:space:]]*;/ && /^[[:space:]]*inclib[[:space:]]+"/' "$_cmdfile" | \
        sed -e 's/.*inclib[[:space:]]*"\([^"]*\)".*/\1/' -e 's/\r$//' > "$_ldir/inclibs.list"
    while IFS= read -r _rellib; do
        [ -n "$_rellib" ] || continue
        _lfile=$(basename "$_rellib" | tr '[:lower:]' '[:upper:]')
        _lname="${_lfile%.LIB}"
        if [ "$BUILD_MODE" = "full-source" ] || [ -f "$BUILD_DIR/libobjs/$_lfile" ]; then
            cp "$BUILD_DIR/libobjs/$_lfile" "$_ldir/l${_li}.lib"
        else
            _lrow=$(awk -F '\t' -v l="$_lname" '!/^#/ && $1==l && $7=="-" { print $3 "\t" $4 "\t" $5; exit }' "$CONFIG_DIR/libraries.tsv")
            _kfile=$(printf '%s\n' "$_lrow" | cut -f1)
            _ksha=$(printf '%s\n' "$_lrow" | cut -f2)
            _klist=$(printf '%s\n' "$_lrow" | cut -f3)
            _kpath=$(find_kit_lib "$_kfile" "$_ksha")
            _kit_libs=$((_kit_libs + 1))
            if [ "$HAS_STATED" = "1" ] && awk -F '\t' -v p="$_prog" -v l="$_lname" '!/^#/ && $1==p && $2==l { f=1; exit } END { exit !f }' "$CONFIG_DIR/stated.tsv"; then
                _pmap="$_ldir/patch_${_lname}.map"
                : > "$_pmap"
                awk -F '\t' -v p="$_prog" -v l="$_lname" '!/^#/ && $1==p && $2==l && !seen[$3]++ { print $3 }' "$CONFIG_DIR/stated.tsv" > "$_ldir/smods_${_lname}.list"
                while IFS= read -r _smod; do
                    [ -n "$_smod" ] || continue
                    _sobj="$_ldir/stated_${_lname}_${_smod}.obj"
                    build_stated_obj "$_prog" "$_lname" "$_smod" "$_exe_in" "$R_AS" "$R_ASOS" "$_sobj"
                    printf '%s\t%s\t%s\n' "$_smod" "$_sobj" "${_sobj}.meta" >> "$_pmap"
                    _stated_mods=$((_stated_mods + 1))
                done < "$_ldir/smods_${_lname}.list"
                _sb=$(awk -F '\t' -v p="$_prog" -v l="$_lname" '!/^#/ && $1==p && $2==l && $7!="-" { sum += $6 + 0 } END { printf "%.0f\n", sum }' "$CONFIG_DIR/stated.tsv")
                _stated_bytes=$((_stated_bytes + _sb))
                patch_kit_lib "$_kpath" "$_ldir/l${_li}.lib" "$_klist" "$_pmap"
            else
                cp "$_kpath" "$_ldir/l${_li}.lib"
            fi
        fi
        printf '\tinclib\t"l%d.lib"\r\n' "$_li" >> "$_ldir/link.lnk"
        _li=$((_li + 1))
    done < "$_ldir/inclibs.list"

    if [ "$_has_order" = "1" ]; then
        printf '\tinclib\t"order.lib"\r\n' >> "$_ldir/link.lnk"
    fi
    awk '!/^[[:space:]]*;/ && /^[[:space:]]*regs[[:space:]]/ { gsub(/\r/, ""); printf "%s\r\n", $0 }' "$_cmdfile" >> "$_ldir/link.lnk"

    (
        cd "$_ldir"
        if ! run_tool "$T_LINK_OS" "$T_LINK" /c /m /n 1024 "@link.lnk,out.cpe,out.sym,out.map" > "psylink.log" 2>&1; then
            cat "psylink.log" >&2
            exit 1
        fi
    )
    cpe_to_exe_fast "$_ldir/out.cpe" "$BUILD_DIR/out/$_exename" "$_marker"
    _gotsha=$(file_sha256 "$BUILD_DIR/out/$_exename")
    if [ "$_gotsha" != "$_wantsha" ]; then
        echo "error: $_prog ($_exename) sha256 mismatch: got $_gotsha, expected $_wantsha" >&2
        exit 1
    fi
    _exesz=$(wc -c < "$BUILD_DIR/out/$_exename" | tr -d ' \r\n')
    if [ "$BUILD_MODE" = "full-source" ]; then
        echo "  $_prog ($_exename): $_gotsha OK ($_oi game units + $LIB_MOD_COUNT library units compiled from source)"
    else
        echo "  $_prog ($_exename): $_gotsha OK ($_oi units + $LIB_MOD_COUNT non-kit library units compiled; $_kit_libs kit libraries; $_stated_mods stated module(s), $_stated_bytes / $_exesz bytes)"
    fi
    if [ -n "$REPORT_DIR" ]; then
        _rep="$CONFIG_DIR/reports/${_exename}.json"
        if [ -f "$_rep" ]; then
            mkdir -p "$REPORT_DIR"
            cp "$_rep" "$REPORT_DIR/${_exename}.json"
            echo "  report: $REPORT_DIR/${_exename}.json"
        fi
    fi
    VERIFIED=$((VERIFIED + 1))
}

VERIFIED=0
build_program 'SLPS' 'SLPS_016.03' 'SLPS_EXE' "${SLPS_EXE:-}" 'config/lain/SLPS.LNK' 'src' 'include' '-' '33e8e8f8ae7eea1a94d392dc851f639090633bcfb037de164ac5c3a39ef02f95' 'Sony Computer Entertainment Inc. for Japan area'

if [ "$VERIFIED" -eq 0 ]; then
    echo "error: no matching program selected ($WANT_BUILDS)" >&2
    exit 1
fi
echo "Verified $VERIFIED executable(s) cleanly."
