import lib


def Setup(id, pw, ip, opt, spdk_dir):
    lib.printer.green(f" + {__name__}.Setup : {opt}")
    run_setup = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo HUGE_EVEN_ALLOC={opt['HUGE_EVEN_ALLOC']} NRHUGE={opt['NRHUGE']} \
        {spdk_dir}/scripts/setup.sh"
    lib.subproc.sync_run(run_setup)
