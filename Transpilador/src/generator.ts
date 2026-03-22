import * as fs from 'fs';
import * as path from 'path';
import { Subsystem } from './Types';

const defaultValues: Record<string, string> = {
    'int': '0',
    'number': '0',
    'string': '""',
    'boolean': 'false',
    'any': 'null'
};

export function generateFiles(subsystems: Subsystem[]) {
    subsystems.forEach(sub => {
        const targetDir = path.resolve(sub.path, sub.name);

        if (!fs.existsSync(targetDir)) {
            fs.mkdirSync(targetDir, { recursive: true });
        }

        sub.classes.forEach(cls => {
            let content = `transpilador \n\n`;
            content += `export class ${cls.name.replace(/\s+/g, '')} {\n\n`;

            cls.methods.forEach(m => {
                const typeLower = m.returnType.toLowerCase();
                const isVoid = typeLower === 'void';
                
                const retValue = defaultValues[typeLower] || 'null';
                
                const tsParams = m.params.split(',').map(p => {
                    const parts = p.trim().split(/\s+/);
                    if (parts.length === 2) {
                        const [type, name] = parts;
                        const finalType = type.toLowerCase() === 'int' ? 'number' : type;
                        return `${name}: ${finalType}`;
                    }
                    return p.trim();
                }).filter(p => p !== "").join(', ');

                const finalReturnType = typeLower === 'int' ? 'number' : m.returnType;

                content += `    ${m.visibility} ${m.name}(${tsParams}): ${finalReturnType} {\n`;
                
                if (isVoid) {
                    content += `        return;\n`; 
                } else {
                    content += `        return ${retValue};\n`;
                }
                
                content += `    }\n\n`;
            });

            content += `}\n`;
            const fileName = `${cls.name.replace(/\s+/g, '')}.ts`;
            const filePath = path.join(targetDir, fileName);
            
            fs.writeFileSync(filePath, content);
            console.log(`[GENERADO] Carpeta: ${sub.name} -> Archivo: ${fileName}`);
        });
    });
}