# Updating tree with overlay

## Compile

```shell
dtc -@ -I dts -O dtb -o rc522_overlay.dtbo rc522_overlay.dts
```

Isso vai gerar o arquivo `rc522_overlay.dtbo`

## Check if overlay is supported

```shell
sudo modprobe configs
```

```shell
zcat /proc/config.gz | grep CONFIGFS
```

Should result in line `CONFIG_CONFIGFS_FS=y`.

```shell
zcat /proc/config.gz | grep OF_OVERLAY
```

Should result in line `CONFIG_OF_OVERLAY=y`.

## Insert tree using configs

```shell
mount -t configfs none /sys/kernel/config
```

```shell
mkdir /sys/kernel/config/device-tree/overlays/rc522
```
```shell
cat rc522_overlay.dtbo > /sys/kernel/config/device-tree/overlays/rc522/dtbo
```

## Remove device-tree overlay

```shell
rmdir /sys/kernel/config/device-tree/overlays/rc522
```