project('macutils', 'c')
# Use the following flags on the CF macro definition as needed.
#
# -DBSD if you are on a BSD system
#
# -DTYPES_H if your system has /usr/include/sys/types.h
#
# -DDIRENT_H if your system has /usr/include/dirent.h
#
# -DTERMIOS_H if your system has /usr/include/sys/termios.h
#
# -DNODOT if you do not want to create files with an initial period
#
# -DLATIN1 if your system supports LATIN-1 and you want to use it
#
# Note you can use at most one of the following four!
#
# -DNOMKDIR if your system does not have the mkdir system call
#
# -DAUFS if you want to use an AUFS file system
#
# -DAUFSPLUS if you use CAP 6.0 and want to use times on files
#
# -DAPPLEDOUBLE if you want to be able to use an AppleDouble file system
#
add_global_arguments([
    '-DNODOT',
    '-DAPPLEDOUBLE'],
    language: 'c')
add_global_arguments(['-Wall'], language: 'c')

compiler = meson.get_compiler('c')

if compiler.has_header('sys/types.h')
    add_global_arguments('-DTYPES_H', language: 'c')
endif
if compiler.has_header('dirent.h')
    add_global_arguments('-DDIRENT_H', language: 'c')
endif
if compiler.has_header('sys/termios.h')
    add_global_arguments('-DTERMIOS_H', language: 'c')
endif


# Static libraries

src_libutil = ['util/backtrans.c', 'util/transname.c', 'util/util.c']
src_libfileio = ['fileio/fileglob.c', 'fileio/rdfile.c', 'fileio/wrfile.c']

libutil = static_library('util', src_libutil)
libfileio = static_library('fileio', src_libfileio)

# CRC
makecrc = executable('makecrc', 'crc/makecrc.c', install: false)
crc_generated = ['arc.c', 'ccitt.c', 'kermit.c', 'binhex.c', 'ccitt32.c', 'zip.c']

gen_crc = custom_target('gen-crc',
    output: crc_generated,
    command: [makecrc])

libcrc = static_library('crc', gen_crc, include_directories: 'crc')


# Executables
binhex = ['binhex/binhex.c', 'binhex/dofile.c']
hexbin = ['hexbin/buffer.c', 'hexbin/crc.c', 'hexbin/dl.c', 'hexbin/globals.c', 'hexbin/hecx.c', 'hexbin/hexbin.c', 'hexbin/hqx.c', 'hexbin/mu.c', 'hexbin/printhdr.c', 'hexbin/readline.c']
macunpack = ['macunpack/bin.c', 'macunpack/bits_be.c', 'macunpack/cpt.c', 'macunpack/crc.c', 'macunpack/dd.c', 'macunpack/de_compress.c', 'macunpack/de_huffman.c', 'macunpack/de_lzah.c', 'macunpack/de_lzh.c', 'macunpack/dia.c', 'macunpack/dir.c', 'macunpack/globals.c', 'macunpack/jdw.c', 'macunpack/lzc.c', 'macunpack/lzh.c', 'macunpack/macbinary.c', 'macunpack/macunpack.c', 'macunpack/mcb.c', 'macunpack/pit.c', 'macunpack/sit.c', 'macunpack/stf.c', 'macunpack/zma.c']
mixed_macsave = ['mixed/dir.c', 'mixed/globals.c', 'mixed/globals.h', 'mixed/macbinary.c', 'mixed/macsave.c', 'mixed/mcb.c']
mixed_macstream = ['mixed/macstream.c']

executable('binhex', binhex, link_with: [libcrc, libutil, libfileio], install: true)
executable('hexbin', hexbin, link_with: [libcrc, libutil, libfileio], install: true)
executable('macunpack', macunpack, link_with: [libcrc, libutil, libfileio], install: true)
executable('macsave', mixed_macsave, link_with: [libutil, libfileio], install: true)
executable('macstream', mixed_macstream, link_with: [libutil, libfileio], install: true)

src_libcomm = ['comm/globals.c', 'comm/tty.c']
libcomm = static_library('comm', src_libcomm)
comm_frommac = ['comm/frommac.c', 'comm/xm_from.c', 'comm/ym_from.c', 'comm/zm_from.c']
comm_tomac = ['comm/tomac.c', 'comm/xm_to.c', 'comm/ym_to.c', 'comm/zm_to.c']
executable('frommac', comm_frommac, link_with: [libcomm, libcrc, libutil, libfileio], install: true)
executable('tomac', comm_tomac, link_with: [libcomm, libcrc, libutil, libfileio], install: true)

# Install

man_pages = ['man/binhex.1','man/frommac.1','man/hexbin.1','man/macsave.1','man/macstream.1','man/macunpack.1','man/macutil.1','man/tomac.1']
install_man(man_pages)
