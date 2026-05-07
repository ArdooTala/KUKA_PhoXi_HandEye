FROM ubuntu:noble

RUN apt update \
    && apt -y upgrade \
    && apt -y --no-install-recommends install \
        cmake make g++ gcc git wget unzip tmux lsb-release \
        gnupg curl software-properties-common python3-venv

RUN apt -y install \
        libopencv-dev libpcl-dev libeigen3-dev
# Install NeoVim
RUN add-apt-repository -y ppa:neovim-ppa/unstable \
    && apt update \
    && apt -y install ripgrep xclip neovim python3-neovim
ENV TERM='xterm-256color'
ADD https://github.com/ArdooTala/my-nvim-config.git /root/.config/nvim/

# Install PhoXiControl
RUN apt -y install avahi-daemon libqt5gui5 libavahi-client-dev

COPY ./PhotoneoPhoXiControlInstaller-1.16.1-Ubuntu24-STABLE.run /root/PhoXiControlInstaller/

ENV PHOXI_CONTROL_PATH="/opt/Photoneo/PhoXiControl"
ENV PATH=${PATH}:${PHOXI_CONTROL_PATH}/bin
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PHOXI_CONTROL_PATH}/API/lib
ENV CPATH=${CPATH}:${PHOXI_CONTROL_PATH}/API/include

RUN /root/PhoXiControlInstaller/PhotoneoPhoXiControlInstaller-1.16.1-Ubuntu24-STABLE.run --accept ${PHOXI_CONTROL_PATH}

# Install aist-phoxi-camera ros package deps
RUN apt -y install nlohmann-json3-dev

RUN apt update && apt -y install dbus-x11
COPY ./entrypoint.sh /root/entrypoint.sh
RUN chmod +x /root/entrypoint.sh
RUN echo "if [ -f /tmp/dbus_session_address ]; then source /tmp/dbus_session_address; fi" >> ~/.bashrc

ENV PHOXI_WITHOUT_DISPLAY=ON

WORKDIR /root

ENTRYPOINT ["/root/entrypoint.sh"]

CMD [ "bash" ]
