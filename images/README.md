# Docker images

This is the image used to build motorZaberMotion on gitlab.

## Commands

Building and pushing the image:

```sh
docker build -f images/epics_ubuntu.Dockerfile -t registry.izaber.com/software-public/zaber-motion-epics/linux_amd64 .
docker push registry.izaber.com/software-public/zaber-motion-epics/linux_amd64
```

You can build for a different platform with the `--platform` flag like:
```
docker build -f images/epics_ubuntu.Dockerfile -t registry.izaber.com/software-public/zaber-motion-epics/linux_amd64 --platform linux/amd64 .
```

### Access Denied

If you see `denied: requested access to the resource is denied`, it means you will need to log in by running:

```
docker login registry.izaber.com -u "yourname@zaber.com"
```

You'll then need to provide an access token to the repo with read_registry, and write_registry permissions.
