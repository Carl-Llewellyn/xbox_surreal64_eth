# 1) Replace smb.conf with a minimal, explicit config
sudo tee /etc/samba/smb.conf >/dev/null <<'EOF'
[global]
   workgroup = WORKGROUP
   server string = CachyOS Samba
   security = user
   map to guest = never
   interfaces = lo virbr0
   bind interfaces only = yes
   smb ports = 445 139
   disable netbios = no

[winshare]
   path = /srv/winshare
   browseable = yes
   read only = no
   guest ok = no
   force user = carl
   create mask = 0664
   directory mask = 2775
EOF

# 2) Restart samba
sudo systemctl restart smb nmb

# 3) Allow samba through firewalld on the libvirt zone
sudo firewall-cmd --get-active-zones
sudo firewall-cmd --zone=libvirt --add-service=samba --permanent
sudo firewall-cmd --reload

