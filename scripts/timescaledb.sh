#!/bin/sh
set -e

CONTAINER_NAME=${1:-"timescale-db-demantoid"}
IMAGE=${2:-"timescale/timescaledb-ha:pg18"}
HOST_PORT=${3:-"16543"}
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
      -e POSTGRES_USER=demo_user \
      -e POSTGRES_PASSWORD=demo_password \
      -e POSTGRES_DB=demantoid \
      -d "${IMAGE}"
fi

echo "URL: <postgresql://demo_user:demo_password@localhost:$HOST_PORT/demantoid>"
