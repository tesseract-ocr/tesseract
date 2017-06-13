# Dockerfile for local Travis build test

FROM ubuntu
MAINTAINER Ian Blenke <ian@blenke.com>

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y cmake curl git ruby bundler wget unzip
RUN gem install bundler travis --no-ri --no-rdoc
RUN git clone --depth 1 https://github.com/travis-ci/travis-build ~/.travis/travis-build
RUN bundle install --gemfile ~/.travis/travis-build/Gemfile

ADD . /tesseract
WORKDIR /tesseract

RUN travis compile | sed -e "s/--branch\\\=\\\'\\\'/--branch=master/g" | bash
