# CTF Works

Tools and scripts for CTF `exploit/pwnable` challenge development.

## Challenge Organization

* Each challenge goes in its own directory in challenges/${challenge}
* Each challenge must be packaged as a `docker` container and must have a `Dockerfile`
* Challenges can share `binaries` or any other file for distribution after packaging through /shared (??)

## Flags

The flag for each challenge will be available from process `environment variable`. The name of the environment variable can be any of the following:

* FLAG
* PWNFLAG
* PFLAG
* FLAG-FOR-PWNS

## Development Environment

Create the virtual machine with `Vagrant`

```
$ vagrant up
```

Login with SSH and create ssh keys

```
$ vagrant ssh
$ ssh-keygen
```

The generated public need to be added to Bitbucket for repository access from inside the development environment. Do not forget to remove the keys from Bitbucket once development environment is no longer required.
