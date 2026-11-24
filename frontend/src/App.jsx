import { useMachines } from './hooks/useMachines'
import MachineGrid from './components/MachineGrid'
import './App.css'

/**
 * Render the RPA Drift application while retaining the last
 * available machine measurements during connection failures.
 *
 * @returns {import('react').ReactElement} The application layout.
 */
function App() {
    const { machines, loading, error } = useMachines()

    return (
        <>
            <header>
                <h1>RPA Drift</h1>
                <p>Overvågning af robotmiljøet</p>
            </header>

            <main>
                <section aria-labelledby="machines-heading">
                    <h2 id="machines-heading">Maskinoversigt</h2>

                    {loading && <p>Henter maskindata...</p>}

                    {error && (
                        <div className="connection-warning" role="alert">
                            <strong>Forbindelsesfejl</strong>
                            <p>
                                {machines.length > 0
                                    ? 'Viser senest hentede maskindata.'
                                    : 'Maskindata kunne ikke hentes.'}
                                {' '}Forsøger igen automatisk.
                            </p>
                        </div>
                    )}

                    {!loading && machines.length > 0 && (
                        <MachineGrid machines={machines} />
                    )}

                    {!loading && !error && machines.length === 0 && (
                        <p>Ingen registrerede maskiner.</p>
                    )}
                </section>
            </main>
        </>
    )
}

export default App