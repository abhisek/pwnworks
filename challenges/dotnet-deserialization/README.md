# .NET Deserialization Attack

## Learning Objective

* .NET Framework
* C# Language and CLR
* Deserialization attacks against .NET Framework

## Deserialization Attacks

The vulnerability exists due to an attacker's ability to load and run `deserialization callback` methods in any arbitrary class in application's classpath. Depending on the `deserialization callbacks` and availability of usable libraries for gadgets, an attacker may execute arbitrary code by exploiting the scenario.

Good example available at:
https://cert.360.cn/warning/detail?id=e689288863456481733e01b093c986b6

For attacking `XML Deserializer`, there are certain constraints suchs as:

* `Type name` should be attacker controlled as `XML Deserializer` seem to validate explicit type (Strict type control)
* Only `public` classes can be targeted during deserialization

## Usage

```bash
$ docker build -t dotnet-serial-pwn .
$ docker run --rm --name dotnet-serial-pwn -p 9000:9000 dotnet-serial-pwn
```

## References

* https://speakerdeck.com/pwntester/attacking-net-serialization
* https://www.blackhat.com/docs/us-17/thursday/us-17-Munoz-Friday-The-13th-Json-Attacks.pdf
* https://media.blackhat.com/bh-us-12/Briefings/Forshaw/BH\_US\_12\_Forshaw\_Are\_You\_My\_Type\_Slides.pdf



