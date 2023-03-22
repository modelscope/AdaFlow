conda activate adaflow-dev

export PKG_CONFIG_PATH=$CONDA_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$LD_LIBRARY_PATH
export LDFLAGS="$LDFLAGS -L$CONDA_PREFIX/lib -Wl,-rpath,$CONDA_PREFIX/lib"
export GI_TYPELIB_PATH="$CONDA_PREFIX/lib/girepository-1.0"

# for MacOS only
export DYLD_LIBRARY_PATH=$CONDA_PREFIX/lib:$DYLD_LIBRARY_PATH