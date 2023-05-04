ARG DEVEL_TAG=latest
FROM adaflow/adaflow-devel-cuda:${DEVEL_TAG}

# build adaflow
ADD . /build/adaflow/
RUN rm -rf adaflow/build && mkdir -p adaflow/build && cd adaflow/build && \
    cmake \
        -DCMAKE_BUILD_TYPE=$ADAFLOW_BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$ADAFLOW_PREFIX \
        -DADAFLOW_USE_CUDA=ON \
        -DADAFLOW_USE_TRT=ON \
         .. && \
    make -j${nproc} && make install && \
    cd .. && cd modules/adaflow-python && \
    pip3 install .

CMD python3 -m unittest discover -s /build/adaflow/modules/adaflow-python/test/ -p *_test.py