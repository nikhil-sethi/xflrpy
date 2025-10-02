## Using docker compose

In the root repository
```bash
CURRENT_USER=$(id -u):$(id -g) docker compose up
```
You should see the GUI come up.

## Build dockerfles

```bash
# The 'base' dockerfile (installs dependencies)
docker build -f docker/Dockerfile.base . -t niksethi/xflrpy:base

# The 'latest' dockerfile (insatlls xflrpy GUI and the pythonclient)
docker build -f docker/Dockerfile.latest . -t niksethi/xflrpy:latest
```

## Push to the Registry

```bash
docker login -u niksethi
docker push niksethi/xflrpy:base # or niksethi/xflrpy:latest
```
