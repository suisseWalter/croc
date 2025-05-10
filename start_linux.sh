  1 #!/bin/sh                                                                   
  2 # Copyright (c) 2024 ETH Zurich and University of Bologna.                  
  3 # Licensed under the Apache License, Version 2.0, see LICENSE for details.  
  4 # SPDX-License-Identifier: Apache-2.0                                       
  5 #                                                                           
  6 # Authors:                                                                  
  7 # - Philippe Sauter <phsauter@iis.ee.ethz.ch>                               
  8 
  9 env UID=$(id -u) GID=$(id -g) docker compose pull pulp-docker
 10 
 11 env UID=$(id -u) GID=$(id -g) docker compose run --rm \
 12   -e PS1="\[\033[01;32m\]osic:\[\033[00m\]\[\033[01;34m\]\w\[\033[00m\] $" \
    \
 13   -e DISPLAY=$DISPLAY \
 14   -v /tmp/.X11-unix:/tmp/.X11-unix \
 15   pulp-docker
    
    
#!/bin/sh
# Copyright (c) 2024 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
#
# Authors:
# - Philippe Sauter <phsauter@iis.ee.ethz.ch>

env UID=$(id -u) GID=$(id -g) docker compose pull pulp-docker

env UID=$(id -u) GID=$(id -g) docker compose run --rm \
  -e PS1="\[\033[01;32m\]osic:\[\033[00m\]\[\033[01;34m\]\w\[\033[00m\] $" \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  pulp-docker
