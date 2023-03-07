FROM ruby:3.2

COPY Gemfile Gemfile.lock ./
RUN bundle config --global frozen 1 \
 && bundle install

WORKDIR /
