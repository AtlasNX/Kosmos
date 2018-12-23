FROM devkitpro/devkita64_devkitarm:20181128

# RUN git clone https://github.com/switchbrew/libnx.git --depth=1 \
#     && cd libnx \
#     && make install

VOLUME [ "/code" ]
WORKDIR /code

CMD [ "make", "-j8" ]
