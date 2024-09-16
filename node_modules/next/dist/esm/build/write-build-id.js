import { promises } from "fs";
import { join } from "path";
import { BUILD_ID_FILE } from "../shared/lib/constants";
export async function writeBuildId(distDir, buildId) {
    const buildIdPath = join(distDir, BUILD_ID_FILE);
    await promises.writeFile(buildIdPath, buildId, "utf8");
}

//# sourceMappingURL=write-build-id.js.map