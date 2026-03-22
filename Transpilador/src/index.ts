import * as fs from 'fs';
import * as path from 'path';
import { parsePseudoCode } from './parser';
import { generateFiles } from './generator';

try {
    const inputPath = path.join(process.cwd(), 'input.txt');
    if (!fs.existsSync(inputPath)) {
        console.error("No se encontro input.txt");
        process.exit(1);
    }

    const input = fs.readFileSync(inputPath, 'utf-8');
    const subsystems = parsePseudoCode(input);
    generateFiles(subsystems);

    console.log("\n completado");
} catch (error) {
    console.error("Error: ", error);
}