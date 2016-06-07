#!/bin/bash

#full backup
cd /home/compass
tar -jcvf /mnt/microbio/pipeline/ELEPHANTWALK-BACKUP/full_backup_R000000$1.tar.bz2 R000000$1 > /mnt/microbio/pipeline/ELEPHANTWALK-BACKUP/full_backup_R000000$1.log
