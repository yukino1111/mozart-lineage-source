# Executables installed directly under the system-as-root /sbin directory are
# not covered by Android's default /system/bin executable mode rules.

[sbin/hw_healthd]
mode: 0755
user: AID_ROOT
group: AID_ROOT
caps: 0

[sbin/oeminfo_nvm_server]
mode: 0755
user: AID_ROOT
group: AID_ROOT
caps: 0

[sbin/teecd]
mode: 0755
user: AID_ROOT
group: AID_ROOT
caps: 0
