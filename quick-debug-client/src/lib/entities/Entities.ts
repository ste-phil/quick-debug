export class ConfigMessage {
    constructor(
        public Key: string,
        public Value: string,
    ) {
    }
}

export abstract class Settings {
    public static Port: number = 8126;
}


export class DataFlow {
    constructor(
        public Name: string,
        public AssignedChartIdx: number,
    ) {
    }

}