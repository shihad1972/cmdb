cmdb
=========

Install prerequisites for the ailsatech.net CMDB software

Requirements
------------

No prerequisites.

Role Configurable Variables
--------------

domain:       Initial DNS domain
do_cmdb_user: Create cmdb user
cmdb_user:    UNIX user; defaults to cmdb
cmdb_group:   UNIX group; defaults to cmdb
cmdb_uid:     UNIX uid of cmdb user; defaults to 145
interface:    Network Interface to configure for DHCP
interfaces:   A dict of network configuration for your interface.
              This will auto configure to 172.26.80.0/24
do_mysql:     Configure mysql on the instance.

Dependencies
------------

A list of other roles hosted on Galaxy should go here, plus any details in regards to parameters that may need to be set for other roles, or variables that are used from other roles.

Example Playbook
----------------

Including an example of how to use your role (for instance, with variables passed in as parameters) is always nice for users too:

    - hosts: servers
      roles:
         - { role: username.rolename, x: 42 }

License
-------

GPLv3

Author Information
------------------

Iain M. Conochie <iain@thargoid.co.uk>
