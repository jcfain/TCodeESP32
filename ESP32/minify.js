import {minify} from 'minify';
import tryToCatch from 'try-to-catch';
import * as fs from 'fs';
import * as path from 'path';
import {glob} from 'glob';
import { createGzip } from 'node:zlib';
import { pipeline } from 'node:stream';
import { promisify } from 'node:util';

const pipelineAsync = promisify(pipeline);

const options = {
    "js": {
        "type": "terser"
    },
    "img": {
        "maxSize": 4096
    },
    "html": {
        "removeComments": true,
        "removeCommentsFromCDATA": true,
        "removeCDATASectionsFromCDATA": true,
        "collapseWhitespace": true,
        "collapseBooleanAttributes": true,
        "removeAttributeQuotes": true,
        "removeRedundantAttributes": true,
        "useShortDoctype": true,
        "removeEmptyAttributes": true,
        "removeEmptyElements": false,
        "removeOptionalTags": true,
        "removeScriptTypeAttributes": true,
        "removeStyleLinkTypeAttributes": true,
        "minifyJS": false,
        "minifyCSS": false
    },
    "css": {
        "type": "clean-css",
        "clean-css": {
            "compatibility": "*"
        }
    }
};

const dry_run = false;

const input_dir = "dataEdit/www";
const output_dir = "data/www";

// --- Bundling configuration ---
// JS files in dependency order (matches original <script defer> order in index.html).
const jsBundleOrder = [
    `${input_dir}/utils.js`,
    `${input_dir}/modal-component/modal-component.js`,
    `${input_dir}/battery.js`,
    `${input_dir}/motion-generator.js`,
    `${input_dir}/bldc-motor.js`,
    `${input_dir}/buttons.js`,
    `${input_dir}/range-slider.js`,
    `${input_dir}/esp-timer-setup.js`,
    `${input_dir}/settings.js`,
    `${input_dir}/pwm-test.js`,
];

// CSS files in order
const cssBundleOrder = [
    `${input_dir}/style.css`,
    `${input_dir}/range-slider.css`,
];

const copyfiles = await glob(`${input_dir}/*.{png,ico,jpg}`);

const extract_fileinfo = (filepath) => {
    filepath = path.normalize(filepath);
    const basename = filepath.split(path.sep).reverse()[0];
    return basename.split('.');
};

async function do_gzip(input, output) {
    const gzip = createGzip();
    const source = fs.createReadStream(input);
    const destination = fs.createWriteStream(output);
    await pipelineAsync(source, gzip, destination);
}

async function writeMinGz(minFile, data) {
    fs.writeFileSync(minFile, data);
    const gzFile = minFile + ".gz";
    console.log(`  GZip ${minFile} to ${gzFile}...`);
    await do_gzip(minFile, gzFile);
    fs.rmSync(minFile);
    console.log(`  Removed uncompressed ${minFile}`);
}

// --- Bundle & minify JS ---
console.log(`\nBundling ${jsBundleOrder.length} JS files...`);
let minifiedJS = '';
{
    let concatenated = '';
    for (const filepath of jsBundleOrder) {
        console.log(`  Reading ${filepath}`);
        concatenated += fs.readFileSync(filepath, 'utf-8') + '\n;\n';
    }
    const tempFile = `${output_dir}/_bundle_temp.js`;
    fs.writeFileSync(tempFile, concatenated);
    const [error, data] = await tryToCatch(minify, tempFile, options);
    fs.rmSync(tempFile);
    if (error) {
        console.error(`JS bundle minify error: ${error}`);
        process.exit(1);
    }
    minifiedJS = data;
    console.log(`  Minified JS bundle: ${minifiedJS.length} bytes`);
}

// --- Bundle & minify CSS ---
console.log(`\nBundling ${cssBundleOrder.length} CSS files...`);
let minifiedCSS = '';
{
    let concatenated = '';
    for (const filepath of cssBundleOrder) {
        console.log(`  Reading ${filepath}`);
        concatenated += fs.readFileSync(filepath, 'utf-8') + '\n';
    }
    const tempFile = `${output_dir}/_bundle_temp.css`;
    fs.writeFileSync(tempFile, concatenated);
    const [error, data] = await tryToCatch(minify, tempFile, options);
    fs.rmSync(tempFile);
    if (error) {
        console.error(`CSS bundle minify error: ${error}`);
        process.exit(1);
    }
    minifiedCSS = data;
    console.log(`  Minified CSS bundle: ${minifiedCSS.length} bytes`);
}

// --- Build single-file HTML with inlined CSS and JS ---
console.log(`\nBuilding single-file HTML...`);
{
    let html = fs.readFileSync(`${input_dir}/index.html`, 'utf-8');

    // Replace the external CSS link with inline <style>
    html = html.replace(
        /\s*<link[^>]*href="style-bundle-min\.css"[^>]*>/i,
        `\n  <style>${minifiedCSS}</style>`
    );

    // Replace the external JS script with inline <script> (no defer needed for inline)
    html = html.replace(
        /\s*<script[^>]*src="bundle-min\.js"[^>]*><\/script>/i,
        ''  // Remove the external script tag; we'll add inline at end of body
    );

    // Insert the JS bundle at the end of <body> so all DOM elements exist
    html = html.replace(
        '</body>',
        `<script>${minifiedJS}</script>\n</body>`
    );

    // Write temp file for HTML minifier
    const tempHtmlFile = `${output_dir}/_temp_index.html`;
    fs.writeFileSync(tempHtmlFile, html);
    const [error, minifiedHtml] = await tryToCatch(minify, tempHtmlFile, options);
    fs.rmSync(tempHtmlFile);

    if (error) {
        console.error(`HTML minify error: ${error}`);
        process.exit(1);
    }

    const outFile = `${output_dir}/index-min.html`;
    console.log(`  Writing ${outFile} (${minifiedHtml.length} bytes)`);
    if (!dry_run) {
        await writeMinGz(outFile, minifiedHtml);
    }
}

await Promise.all(copyfiles.map(async (filepath) => {
    console.log(`Copying ${filepath} to ${output_dir}`);
    const [name, ext] = extract_fileinfo(filepath);

    if (!dry_run)
    {
        return fs.copyFile(filepath, `${output_dir}/${name}.${ext}`, (err) => {
            if (err) {
                console.error(`Copy error (${filepath}): ${err}`);
            }
        });
    } else {
        return Promise.resolve();
    }
}));