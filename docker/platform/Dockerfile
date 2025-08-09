
# define global args
ARG base_image=timbre:latest
ARG tempo_image=tempo:latest
ARG lyric_image=lyric:latest

# declare the tempo named context
FROM ${tempo_image} AS tempo

# declare the lyric named context
FROM ${lyric_image} AS lyric

# declare the base stage
FROM ${base_image} AS base

# define stage args
ARG is_debug_build=0

# switch current user
USER jrandomhacker

# copy the tempo cache save from the tempo image
COPY --from=tempo /home/jrandomhacker/src/tempo/tempo_cache_save.tgz /tmp/

# load the tempo package into the conan cache
RUN conan cache restore /tmp/tempo_cache_save.tgz

# copy the lyric cache save from the tempo image
COPY --from=lyric /home/jrandomhacker/src/lyric/lyric_cache_save.tgz /tmp/

# load the lyric package into the conan cache
RUN conan cache restore /tmp/lyric_cache_save.tgz

# copy zuri repository into the image
ADD --chown=jrandomhacker . /home/jrandomhacker/src/zuri

# change to zuri repository directory
WORKDIR /home/jrandomhacker/src/zuri

# build zuri in the conan cache
RUN conan create .

# create a conan cache save containing only zuri
RUN conan cache save "zuri/*:*" --file zuri_cache_save.tgz

# remove zuri temporary files
RUN conan cache clean --download --temp --backup-sources 'zuri/*'

# if this is not a debug build then remove zuri build files
ENV IS_DEBUG_BUILD=$is_debug_build
RUN if test $IS_DEBUG_BUILD == 0; then conan cache clean --source --build 'zuri/*'; fi
