import {minify} from 'minify';
import tryToCatch from 'try-to-catch';
import * as fs from 'fs';
import * as path from 'path';
import {glob} from 'glob';
import { createGzip } from 'node:zlib';
import { pipeline } from 'node:stream';

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

const minfiles = await glob([`${input_dir}/*.{js,css,html}`, `${input_dir}/**/*.{js,css,html}`]);
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
  await pipeline(source, gzip, destination, (err) => {
        if (err) {
            console.error('An pipeline error occurred:', err);
        }
    });
}

await Promise.all(minfiles.map(async (filepath) => {
    const [name, ext] = extract_fileinfo(filepath);
    console.log(`Minifying ${filepath} to ${output_dir}/${name}-min.${ext}... `)
    const [error, data] = await tryToCatch(minify, filepath, options);
    if (!error) {
       
        if (!dry_run)
        { 
            const minFile = `${output_dir}/${name}-min.${ext}`;
            fs.writeFile(minFile, data, {}, (err) => {
                if (err) {
                    console.error('Minify error:', err)
                    return;
                }
                const gzFile = minFile+".gz";
                console.log(`GZip ${minFile} to ${gzFile}... `)
                do_gzip(minFile, gzFile)
                    .catch((err) => {
                        console.error('An error occurred gzipping:', err);
                    }).finally(() => {
                        console.log(`Remove ${minFile}`)
                        fs.rm(minFile, () => {
                            console.log(`Removed`)
                        });
                    });
            });
        } else {
            return Promise.resolve();
        }
    } else {
        console.error(`Err (${filepath}): ${error}`);
    }
}));

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