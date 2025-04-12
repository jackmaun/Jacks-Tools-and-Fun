namespace GroverHashCracker {
    open Microsoft.Quantum.Intrinsic;
    open Microsoft.Quantum.Canon;

    operation Diffuser(qs : Qubit[]) : Unit is Adj {
        within {
            ApplyToEachA(H, qs);
            ApplyToEachA(X, qs);
        } apply {
            Controlled Z(Most(qs), Tail(qs));
        }
        ApplyToEachA(X, qs);
        ApplyToEachA(H, qs);
    }

    operation GroverSearch(target : Bool[], hash_fn : (Qubit[] => Unit is Adj)) : Result[] {
        use qs = Qubit[Length(target)];

        ApplyToEach(H, qs);
        let iterations = 2;

        for (_ in 1..iterations) {
            Oracle(qs, target, hash_fn);
            Diffuser(qs);
        }

        return ForEach(MResetZ, qs);
    }
}
