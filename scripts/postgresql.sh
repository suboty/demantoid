#!/bin/sh
set -e

CONTAINER_NAME=${1:-"postgres-db-demantoid"}
IMAGE=${2:-"postgres"}
HOST_PORT=${3:-"15432"}
CONTAINER_PORT=${4:-"5432"}

if [ -n "$(docker ps -a -q -f name=^/${CONTAINER_NAME}$)" ]; then
    if [ -z "$(docker ps -q -f name=^/${CONTAINER_NAME}$)" ]; then
        echo "Starting existing container ${CONTAINER_NAME}..."
        docker start "${CONTAINER_NAME}"
    else
        echo "Container ${CONTAINER_NAME} is already running."
    fi
else
    echo "Creating container ${CONTAINER_NAME}..."
    docker run \
      --name "${CONTAINER_NAME}" \
      -p "${HOST_PORT}:${CONTAINER_PORT}" \
      -e POSTGRES_USER=demantoid \
      -e POSTGRES_PASSWORD=demantoid_password \
      -e POSTGRES_DB=demantoid \
      -d "${IMAGE}"
fi
