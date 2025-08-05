
# define global args
ARG zuri_image=zuri:latest

# declare the zuri named context
FROM ${zuri_image} AS zuri

# declare the base stage
FROM scratch

# copy the cache save
COPY --from=zuri /home/jrandomhacker/src/zuri/zuri_cache_save.tgz /home/jrandomhacker/src/zuri/
