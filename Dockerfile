FROM ubuntu@sha256:669e010b58baf5beb2836b253c1fd5768333f0d1dbcb834f7c07a4dc93f474be
SHELL ["/bin/bash","-c"]
ENV PROJECT_NAME="Opengl" \
    DISPLAY=host.docker.internal:0.0
WORKDIR /src
RUN apt-get update && apt-get install -y cmake make g++ libx11-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxrandr-dev libxext-dev libxinerama-dev libxcursor-dev xterm x11-xserver-utils
COPY . /src/
RUN chmod 777 /src/buildrun.sh
RUN mkdir build && cd build/ && cmake .. -G "Unix Makefiles" && make all
CMD /src/buildrun.sh