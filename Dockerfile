# Dockerfile 
# docker build -t tank .
# docker run -it -v /dev:/dev --device /dev --rm tank 

######################
ARG USERNAME=root
ARG USER_UID=1000
ARG USER_GID=$USER_UID
######################

#########################################
### Ubuntu Distribution as Base Image ###
#########################################
FROM ubuntu:22.04 as base

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update \ 
    && apt-get install -y \
    sudo \
    vim \
    make \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# create workspace
RUN mkdir -p /home/ws/src
RUN mkdir -p /home/ws/include
WORKDIR /home/ws

# Copy in files
COPY controller/ /home/ws/
COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT [ "/bin/bash", "/entrypoint.sh" ]

CMD ["bash", "-l"]