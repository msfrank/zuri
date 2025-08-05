
# declare the tempo named context
FROM ghcr.io/msfrank/tempo-pkgcache-debug:latest AS tempo

# declare the lyric named context
FROM ghcr.io/msfrank/lyric-pkgcache-debug:latest AS lyric

# declare the base stage
FROM ghcr.io/msfrank/timbre-debug:latest AS base

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