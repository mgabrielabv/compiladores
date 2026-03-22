import { Subsystem, Class, Method } from './Types';

export function parsePseudoCode(input: string): Subsystem[] {
    const subsystemBlocks = input.split(/@subsystem/i).filter(s => s.trim() !== "");
    
    return subsystemBlocks.map(block => {
        const pathMatch = block.match(/path:\s*(.+)/i);
        const nameMatch = block.match(/name:\s*(.+)/i);

        const subsystem: Subsystem = {
            path: pathMatch ? pathMatch[1].trim() : './output',
            name: nameMatch ? nameMatch[1].trim() : 'default_sub',
            classes: []
        };

        const classBlocks = block.split(/class:/i).slice(1);
        classBlocks.forEach(classBlock => {
            const lines = classBlock.split('\n');
            const className = lines[0].trim().replace(/\s+/g, '');
            const methods: Method[] = [];
            let currentVisibility = 'public';

            lines.forEach(line => {
                const cleanLine = line.trim();
                
                if (cleanLine.toLowerCase().match(/^(public|private|protected):$/)) {
                    currentVisibility = cleanLine.replace(':', '').toLowerCase();
                }

                const methodMatch = cleanLine.match(/(\w+)\s*\((.*)\)\s*:\s*(\w+)/);
                if (methodMatch) {
                    methods.push({
                        name: methodMatch[1],
                        params: methodMatch[2],
                        returnType: methodMatch[3],
                        visibility: currentVisibility
                    });
                }
            });
            subsystem.classes.push({ name: className, methods });
        });
        return subsystem;
    });
}