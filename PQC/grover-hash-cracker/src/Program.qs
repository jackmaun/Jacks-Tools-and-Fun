namespace GroverHashCracker {
    open Microsoft.Quantum.Canon;
    open Microsoft.Quantum.Intrinsic;
    open Microsoft.Quantum.Diagnostics;

    @EntryPoint()
    operation Main() : Unit {
        let target = [false, true, true]; // i.e. 0b011 (result of FakeHash(110))

        let result = GroverSearch(target, FakeHash);
        Message($"Grover result: {ResultArrayAsString(result)}");
    }
}
