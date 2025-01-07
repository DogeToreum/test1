FROM debian:stretch
LABEL maintainer="Dogetoreum Developers <dev@dogetoreum.org>"
LABEL description="Dockerised DogetoreumCore, built from Travis"

RUN apt-get update && apt-get -y upgrade && apt-get clean && rm -fr /var/cache/apt/*

COPY bin/* /usr/bin/
