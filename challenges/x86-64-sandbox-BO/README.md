# Sandbox Escape

## Learning Objective

* Linux seccomp sandbox
* Attacking process outside sandbox

## Description

## Flag Store

Due to the nature of this vulnerability, flag cannot be placed in environment variable because it can be read directly off memory. The bootstrap script will clear the environment after placing the flag in `/flag.txt`. 

> Notes

* Solutions need to read the content of `/flag.txt`
* Container orchestrator must place the flag in `PWNFLAG` environment variable.

## Usage

```bash
$ docker build -t exp-sandbox .
$ docker run --rm --name exp-sandbox -p 9000:9000 exp-sandbox
```

## References


