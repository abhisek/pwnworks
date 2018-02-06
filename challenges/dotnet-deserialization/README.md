# .NET Deserialization Attack

## Learning Objective

* .NET Framework
* C# Language and CLR
* Deserialization attacks against .NET Framework

## Deserialization Attacks

The vulnerability exists due to an attacker's ability to load and run `deserialization callback` methods in any arbitrary class in application's classpath. Depending on the `deserialization callbacks` and availability of usable libraries for gadgets, an attacker may execute arbitrary code by exploiting the scenario.



## References

* https://speakerdeck.com/pwntester/attacking-net-serialization
* https://www.blackhat.com/docs/us-17/thursday/us-17-Munoz-Friday-The-13th-Json-Attacks.pdf
* https://media.blackhat.com/bh-us-12/Briefings/Forshaw/BH\_US\_12\_Forshaw\_Are\_You\_My\_Type\_Slides.pdf



