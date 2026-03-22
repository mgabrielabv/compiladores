export interface Method {
    name: string;
    params: string;
    returnType: string;
    visibility: string;
}

export interface Class {
    name: string;
    methods: Method[];
}

export interface Subsystem {
    path: string;
    name: string;
    classes: Class[];
}