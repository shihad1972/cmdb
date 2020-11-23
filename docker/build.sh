#!/bin/bash
#
# Bash script to build container and push into ECR repository
#
# Repository name much match the directory name containing Dockerfile
#
# (C) Iain M. Conochie 2020 (Year of the COVID)
#

if [ -z "$(which aws)" ]; then
  echo "Cannot find aws command in PATH $PATH"
  exit 1
fi

if [ -z "$(which docker)" ]; then
  echo "Cannot find docker command in PATH $PATH"
  exit 1
fi

if [ -z "$(which jq)" ]; then
  echo "Cannot find jq command in PATH $PATH"
  exit 1
fi

while getopts "a:t:" opt; do
  case $opt in
    a ) APP=$OPTARG
        ;;
    t ) TAG=$OPTARG
        ;;
    \?) echo "Usage: $0 -a <APPLICATION> -t <TAG>"
        exit 1;
        ;;
  esac
done

if [ -z "${APP}" ]; then
  echo "APP not set! Defaulting to cmdb"
  APP=cmdb
fi

if [ -z "${TAG}" ]; then
  echo "TAG not set! Defaulting to latest"
  TAG="latest"
fi

if [ -z "$region" ]; then
  region=eu-west-1
fi

host=docker.shihad.org:5000
image=${host}/${APP}:${TAG}
password=$(aws secretsmanager get-secret-value --region eu-west-1 --secret-id /shihad/docker | jq --raw-output '.SecretString' | jq -r .password)

docker login ${host} --password="${password}" --username=iain
echo "Deploying image $image:"
docker build -t $image $APP
docker push $image
