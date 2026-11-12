// Generates the same V8 startup snapshot that obscura-js/build.rs produces on
// a native build, but executed on an Android/arm64 device. The cross build
// cannot run V8's bundled mksnapshot with obscura's bootstrap.js, because the
// bootstrap expects the Deno.core bridge and the Web API globals that only
// deno_core + obscura install at runtime. Building the snapshot on the target
// keeps the heap layout identical to the runtime V8.
//
// Usage: obscura-snapshot [output-path]   (default: OBSCURA_SNAPSHOT.bin)

use std::path::PathBuf;

fn main() {
    let out = std::env::args_os()
        .nth(1)
        .map(PathBuf::from)
        .unwrap_or_else(|| PathBuf::from("OBSCURA_SNAPSHOT.bin"));

    let bootstrap_js = include_str!("../../../crates/obscura-js/js/bootstrap.js")
        .replace(
            "/* INDEXED_DB */",
            include_str!("../../../crates/obscura-js/js/indexed_db.js"),
        );
    assert!(
        bootstrap_js.is_ascii(),
        "bootstrap source must be ASCII; use Unicode escapes in literals"
    );

    let output = deno_core::snapshot::create_snapshot(
        deno_core::snapshot::CreateSnapshotOptions {
            cargo_manifest_dir: env!("CARGO_MANIFEST_DIR"),
            startup_snapshot: None,
            skip_op_registration: true,
            extensions: vec![deno_core::Extension {
                name: "obscura_bootstrap",
                js_files: vec![deno_core::ExtensionFileSource::new_computed(
                    "<obscura:bootstrap>",
                    bootstrap_js.into(),
                )]
                .into(),
                ..Default::default()
            }],
            extension_transpiler: None,
            with_runtime_cb: Some(Box::new(|runtime| {
                runtime
                    .execute_script(
                        "<obscura:snapshot-check>",
                        "if ('__obscura_snapshot_compile' in globalThis) throw Error('Snapshot compiler hook was retained');",
                    )
                    .expect("snapshot compiler hook must not reach page realms");
            })),
        },
        None,
    )
    .expect("Failed to create V8 snapshot");

    std::fs::write(&out, &*output.output).expect("Failed to write snapshot");
    println!(
        "wrote {} ({} bytes)",
        out.display(),
        output.output.len()
    );
}
