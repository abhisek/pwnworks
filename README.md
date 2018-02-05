

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
